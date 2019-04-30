#!/usr/bin/env python3
#
# This work is licensed under the GNU GPLv2 or later.
# See the COPYING file in the top-level directory.

import os
import sys
import pytest
import util


DATADIR = os.environ.get("datadir")
SYSCONFDIR = os.environ.get("sysconfdir")
FOOBAR_DIR = "/foo/bar"


def test_osinfo_db_path_system():
    """
    Test osinfo-db-path --system
    """
    if "OSINFO_SYSTEM_DIR" in os.environ:
        del os.environ["OSINFO_SYSTEM_DIR"]
    cmd = [util.Tools.db_path, util.ToolsArgs.SYSTEM]
    output = util.get_output(cmd)
    expected_output = os.path.join(DATADIR, "osinfo\n")
    assert output == expected_output


def test_osinfo_db_path_local():
    """
    Test osinfo-db-path --local
    """
    if "OSINFO_DATA_DIR" in os.environ:
        del os.environ["OSINFO_DATA_DIR"]
    cmd = [util.Tools.db_path, util.ToolsArgs.LOCAL]
    output = util.get_output(cmd)
    expected_output = os.path.join(SYSCONFDIR, "osinfo\n")
    assert output == expected_output


def test_osinfo_db_path_user():
    """
    Test osinfo-db-path --user
    """
    if "OSINFO_USER_DIR" in os.environ:
        del os.environ["OSINFO_USER_DIR"]
    cmd = [util.Tools.db_path, util.ToolsArgs.USER]
    output = util.get_output(cmd)
    expected_output = os.path.join(os.environ["HOME"], ".config",
                                   "osinfo\n")
    assert output == expected_output


def test_osinfo_db_path_dir():
    """
    Test osinfo-db-path --dir
    """
    cmd = [util.Tools.db_path, util.ToolsArgs.DIR, FOOBAR_DIR]
    output = util.get_output(cmd)
    expected_output = FOOBAR_DIR + "\n"
    assert output == expected_output


def test_osinfo_db_path_root():
    """
    Test osinfo-db-path --root FOOBAR_DIR --system
    """
    if "OSINFO_SYSTEM_DIR" in os.environ:
        del os.environ["OSINFO_SYSTEM_DIR"]
    cmd = [util.Tools.db_path, util.ToolsArgs.ROOT, FOOBAR_DIR,
           util.ToolsArgs.SYSTEM]
    output = util.get_output(cmd)
    expected_output = os.path.join(FOOBAR_DIR, *DATADIR.split("/"), "osinfo\n")
    assert output == expected_output


if __name__ == "__main__":
    exit(pytest.main(sys.argv))
