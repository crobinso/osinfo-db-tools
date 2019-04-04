#!/usr/bin/env python3
#
# This work is licensed under the GNU GPLv2 or later.
# See the COPYING file in the top-level directory.

import os
import shutil
import sys
import pytest
import util


def test_osinfo_db_validate_system():
    """
    Test osinfo-db-validate --system
    """
    os.environ["OSINFO_SYSTEM_DIR"] = util.Data.positive
    cmd = [util.Tools.db_validate, util.ToolsArgs.SYSTEM]
    returncode = util.get_returncode(cmd)
    assert returncode == 0


def test_osinfo_db_validate_local():
    """
    Test osinfo-db-validate --local
    """
    os.environ["OSINFO_LOCAL_DIR"] = util.Data.positive
    cmd = [util.Tools.db_validate, util.ToolsArgs.LOCAL]
    returncode = util.get_returncode(cmd)
    assert returncode == 0


def test_osinfo_db_validate_user():
    """
    Test osinfo-db-validate --user
    """
    os.environ["OSINFO_USER_DIR"] = util.Data.positive
    cmd = [util.Tools.db_validate, util.ToolsArgs.USER]
    returncode = util.get_returncode(cmd)
    assert returncode == 0


def test_osinfo_db_validate_dir():
    """
    Test osinfo-db-validate --dir
    """
    cmd = [util.Tools.db_validate, util.ToolsArgs.DIR, util.Data.positive]
    returncode = util.get_returncode(cmd)
    assert returncode == 0


def test_osinfo_db_validate_root():
    """
    Test osinfo-db-validate --dir
    """
    os.environ["OSINFO_SYSTEM_DIR"] = "positive"
    tempdir = util.tempdir()
    shutil.copytree(util.Data.positive, os.path.join(tempdir, "positive"))
    cmd = [util.Tools.db_validate, util.ToolsArgs.ROOT, tempdir,
           util.ToolsArgs.SYSTEM]
    returncode = util.get_returncode(cmd)
    shutil.rmtree(tempdir)
    assert returncode == 0


def test_negative_osinfo_db_validate_dir():
    """
    Test failure on osinfo-db-validate --dir
    """
    cmd = [util.Tools.db_validate, util.ToolsArgs.DIR, util.Data.negative]
    returncode = util.get_returncode(cmd)
    assert returncode == 1


if __name__ == "__main__":
    exit(pytest.main(sys.argv))
