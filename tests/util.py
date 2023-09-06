# This work is licensed under the GNU GPLv2 or later.
# See the COPYING file in the top-level directory

from enum import EnumMeta
import os
import subprocess
import tempfile


class _Tools():
    """
    Helper class to get the tools binaries
    """
    def __init__(self):
        self.toolsdir = os.path.join(os.environ.get("abs_top_builddir"),
                                     "tools")
        self._db_export = None
        self._db_import = None
        self._db_path = None
        self._db_validate = None

    @property
    def db_export(self):
        """
        Get osinfo-db-export binary
        """
        if not self._db_export:
            self._db_export = os.path.join(self.toolsdir, "osinfo-db-export")
        return self._db_export

    @property
    def db_import(self):
        """
        Get osinfo-db-import binary
        """
        if not self._db_import:
            self._db_import = os.path.join(self.toolsdir, "osinfo-db-import")
        return self._db_import

    @property
    def db_path(self):
        """
        Get osinfo-db-path binary
        """
        if not self._db_path:
            self._db_path = os.path.join(self.toolsdir, "osinfo-db-path")
        return self._db_path

    @property
    def db_validate(self):
        """
        Get osinfo-db-validate binary
        """
        if not self._db_validate:
            self._db_validate = os.path.join(self.toolsdir,
                                             "osinfo-db-validate")
        return self._db_validate


Tools = _Tools()


class _Data():
    """
    Helper class to get the specific data used in the tests
    """
    def __init__(self):
        self.datadir = os.path.join(os.environ.get("abs_top_srcdir"), "tests",
                                    "data")
        self._license = None
        self._positive_data = None
        self._negative_data = None

    @property
    def license(self):
        """
        Get the path to the license file
        """
        if not self._license:
            self._license = os.path.join(self.datadir, "license")
        return self._license

    @property
    def positive(self):
        """
        Get the path for the tests data
        """
        if not self._positive_data:
            self._positive_data = os.path.join(self.datadir, "positive")
        return self._positive_data

    @property
    def negative(self):
        """
        Get the path for the negative tests data
        """
        if not self._negative_data:
            self._negative_data = os.path.join(self.datadir, "negative")
        return self._negative_data


Data = _Data()


def tempdir():
    """
    Create a temporary directory to be used during the tests
    """
    tmpdir = tempfile.mkdtemp(prefix="osinfo-db-tools-tests-")
    return tmpdir


def get_output(cmd):
    """
    Get the stdout output from a command execution
    """
    child = subprocess.Popen(cmd, stdout=subprocess.PIPE)

    stdout, _ = child.communicate()
    return stdout.decode()


def get_returncode(cmd1, cmd2=None):
    """
    Get the return code from a command execution
    """
    cmd1_proc = subprocess.Popen(cmd1, stdout=subprocess.PIPE)
    if cmd2 is not None:
        cmd2_proc = subprocess.Popen(cmd2, stdin=cmd1_proc.stdout)
        cmd2_proc.wait()
    cmd1_proc.wait()

    return cmd2_proc.returncode if cmd2 is not None else cmd1_proc.returncode


class ToolsArgs(EnumMeta):
    SYSTEM = "--system"
    LOCAL = "--local"
    USER = "--user"
    DIR = "--dir"
    ROOT = "--root"
    # --license is only valid for osinfo-db-export
    LICENSE = "--license"
    VERSION = "--version"
    # --latest && --nightly are only valid for osinfo-db-import
    LATEST = "--latest"
    NIGHTLY = "--nightly"
