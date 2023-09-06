#!/usr/bin/env python3
#
# This work is licensed under the GNU GPLv2 or later.
# See the COPYING file in the top-level directory.

import datetime
import filecmp
import json
import os
import shutil
import sys
import pytest
import requests
import util


def test_negative_osinfo_db_export_path():
    """
    Test osinfo-db-export dir fails
    """
    cmd = [util.Tools.db_export, util.Data.positive]
    returncode = util.get_returncode(cmd)
    assert returncode == 1


def test_osinfo_db_export_import_system():
    """
    Test osinfo-db-export --system and osinfo-db-import --system
    """
    # We build the expected filename before running osinfo-db-export;
    # the filename includes the day, so if the day changes while
    # osinfo-db-export runs then we cannot find the output archive
    # anymore.
    # As workaround, build the filename for today and tomorrow,
    # checking that one of them must exist.
    today = datetime.date.today()
    tomorrow = today + datetime.timedelta(days=1)
    default_filename_today = "osinfo-db-%s.tar.xz" % today.strftime("%Y%m%d")
    default_filename_tomorrow = ("osinfo-db-%s.tar.xz" %
                                 tomorrow.strftime("%Y%m%d"))

    os.environ["OSINFO_SYSTEM_DIR"] = util.Data.positive
    cmd = [util.Tools.db_export, util.ToolsArgs.SYSTEM]
    returncode = util.get_returncode(cmd)
    assert returncode == 0
    assert os.path.isfile(default_filename_today) or \
        os.path.isfile(default_filename_tomorrow)
    if os.path.isfile(default_filename_today):
        default_filename = default_filename_today
    else:
        default_filename = default_filename_tomorrow

    tempdir = util.tempdir()
    os.environ["OSINFO_SYSTEM_DIR"] = tempdir
    cmd = [util.Tools.db_import, util.ToolsArgs.SYSTEM, default_filename]
    returncode = util.get_returncode(cmd)
    assert returncode == 0
    dcmp = filecmp.dircmp(util.Data.positive, tempdir)
    assert len(dcmp.right_only) == 1
    assert "VERSION" in dcmp.right_only
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    os.unlink(default_filename)


@pytest.fixture
def osinfo_db_export_local():
    filename = "foobar.tar.xz"

    os.environ["OSINFO_LOCAL_DIR"] = util.Data.positive
    cmd = [util.Tools.db_export, util.ToolsArgs.LOCAL, filename]
    return filename, util.get_returncode(cmd)


@pytest.mark.usefixtures("osinfo_db_export_local")
def test_osinfo_db_export_local(osinfo_db_export_local):
    """
    Test osinfo-db-export --local FILENAME
    """
    filename, returncode = osinfo_db_export_local

    assert returncode == 0
    assert os.path.isfile(filename)


@pytest.mark.usefixtures("osinfo_db_export_local")
def test_osinfo_db_import_local_file(osinfo_db_export_local):
    """
    Test osinfo-db-import --local FILENAME
    """
    filename, _ = osinfo_db_export_local

    tempdir = util.tempdir()
    os.environ["OSINFO_LOCAL_DIR"] = tempdir
    cmd = [util.Tools.db_import, util.ToolsArgs.LOCAL, filename]
    returncode = util.get_returncode(cmd)
    assert returncode == 0
    dcmp = filecmp.dircmp(util.Data.positive, tempdir)
    assert len(dcmp.right_only) == 1
    assert "VERSION" in dcmp.right_only
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    os.unlink(filename)


@pytest.mark.usefixtures("osinfo_db_export_local")
def test_osinfo_db_import_local_dash(osinfo_db_export_local):
    """
    Test cat FILENAME | osinfo-db-import -
    """
    filename, _ = osinfo_db_export_local

    tempdir = util.tempdir()
    os.environ["OSINFO_LOCAL_DIR"] = tempdir
    cmd = ["cat", filename]
    cmd2 = [util.Tools.db_import, util.ToolsArgs.LOCAL, "-"]
    returncode = util.get_returncode(cmd, cmd2)
    assert returncode == 0
    dcmp = filecmp.dircmp(util.Data.positive, tempdir)
    assert len(dcmp.right_only) == 1
    assert "VERSION" in dcmp.right_only
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    os.unlink(filename)


@pytest.mark.usefixtures("osinfo_db_export_local")
def test_osinfo_db_import_local_no_file(osinfo_db_export_local):
    """
    Test cat FILENAME | osinfo-db-import
    """
    filename, _ = osinfo_db_export_local

    tempdir = util.tempdir()
    os.environ["OSINFO_LOCAL_DIR"] = tempdir
    cmd1 = ["cat", filename]
    cmd2 = [util.Tools.db_import, util.ToolsArgs.LOCAL]
    returncode = util.get_returncode(cmd1, cmd2)
    assert returncode == 0
    dcmp = filecmp.dircmp(util.Data.positive, tempdir)
    assert len(dcmp.right_only) == 1
    assert "VERSION" in dcmp.right_only
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    os.unlink(filename)


@pytest.fixture
def osinfo_db_export_user_license_version():
    version = "foobar"
    versioned_filename = "osinfo-db-%s.tar.xz" % version

    os.environ["OSINFO_USER_DIR"] = util.Data.positive
    cmd = [util.Tools.db_export, util.ToolsArgs.USER,
           util.ToolsArgs.LICENSE, util.Data.license,
           util.ToolsArgs.VERSION, version]
    return versioned_filename, version, util.get_returncode(cmd)


@pytest.mark.usefixtures("osinfo_db_export_user_license_version")
def test_osinfo_db_export_user_license_version(
        osinfo_db_export_user_license_version):
    """
    Test osinfo-db-export --user --license LICENSE --version VERSION
    """
    filename, _, returncode = osinfo_db_export_user_license_version

    assert returncode == 0
    assert os.path.isfile(filename)


@pytest.mark.usefixtures("osinfo_db_export_user_license_version")
def test_osinfo_db_import_root_user_versioned_file(
        osinfo_db_export_user_license_version):
    """
    Test osinfo-db-import --root / --user VERSIONED_FILE
    """
    filename, version, returncode = osinfo_db_export_user_license_version

    tempdir = util.tempdir()
    os.environ["OSINFO_USER_DIR"] = tempdir
    cmd = [util.Tools.db_import, util.ToolsArgs.ROOT, "/",
           util.ToolsArgs.USER, filename]
    returncode = util.get_returncode(cmd)
    assert returncode == 0
    dcmp = filecmp.dircmp(util.Data.positive, tempdir)
    assert len(dcmp.right_only) == 2
    assert "VERSION" in dcmp.right_only
    with open(os.path.join(tempdir, "VERSION")) as out:
        content = out.read()
        assert content == version
    assert "LICENSE" in dcmp.right_only
    fcmp = filecmp.cmp(os.path.join(tempdir, "LICENSE"), util.Data.license)
    assert fcmp is True
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    os.unlink(filename)


@pytest.mark.skipif(os.environ.get("OSINFO_DB_TOOLS_NETWORK_TESTS") is None,
                    reason="Network related tests are not enabled")
def test_osinfo_db_import_url():
    """
    Test osinfo-db-import URL
    """
    url = "https://releases.pagure.org/libosinfo/osinfo-db-20190304.tar.xz"

    tempdir = util.tempdir()
    os.environ["OSINFO_USER_DIR"] = tempdir
    cmd = [util.Tools.db_import, util.ToolsArgs.USER, url]
    returncode = util.get_returncode(cmd)
    assert returncode == 0

    test_file = "downloaded.tar.xz"
    tempdir2 = util.tempdir()
    req = requests.get(url)
    open(test_file, "wb").write(req.content)
    assert os.path.isfile(test_file)

    cmd = [util.Tools.db_import, util.ToolsArgs.DIR, tempdir2, test_file]
    returncode = util.get_returncode(cmd)
    assert returncode == 0

    dcmp = filecmp.dircmp(tempdir, tempdir2)
    assert dcmp.right_only == []
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    shutil.rmtree(tempdir2)
    os.unlink(test_file)


@pytest.mark.parametrize(
    "import_net_opt, import_url",
    [
        pytest.param(util.ToolsArgs.LATEST,
                     "https://db.libosinfo.org/latest.json",
                     id="latest"),
        pytest.param(util.ToolsArgs.NIGHTLY,
                     "https://db.libosinfo.org/nightly.json",
                     id="nightly"),
    ]
)
@pytest.mark.skipif(os.environ.get("OSINFO_DB_TOOLS_NETWORK_TESTS") is None,
                    reason="Network related tests are not enabled")
def test_osinfo_db_import_from_net(import_net_opt, import_url):
    """
    Test osinfo-db-import --latest && --nightly
    """
    tempdir = util.tempdir()
    os.environ["OSINFO_USER_DIR"] = tempdir
    cmd = [util.Tools.db_import, import_net_opt]
    returncode = util.get_returncode(cmd)
    assert returncode == 0

    req = requests.get(import_url)
    data = json.loads(req.content)
    url = data["release"]["archive"]

    test_file = "downloaded.tar.xz"
    tempdir2 = util.tempdir()
    req = requests.get(url)
    open(test_file, "wb").write(req.content)
    assert os.path.isfile(test_file)

    cmd = [util.Tools.db_import, util.ToolsArgs.DIR, tempdir2, test_file]
    returncode = util.get_returncode(cmd)
    assert returncode == 0

    dcmp = filecmp.dircmp(tempdir, tempdir2)
    assert dcmp.right_only == []
    assert dcmp.left_only == []
    assert dcmp.diff_files == []
    shutil.rmtree(tempdir)
    shutil.rmtree(tempdir2)
    os.unlink(test_file)


if __name__ == "__main__":
    exit(pytest.main(sys.argv))
