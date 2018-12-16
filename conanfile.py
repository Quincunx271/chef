from conans import ConanFile, CMake, tools
from conans.errors import ConanInvalidConfiguration

import os
import re

def parse_cmakelists(regex, from_match):
    try:
        cmakelists = tools.load('CMakeLists.txt')
        data = from_match(re.search(regex, cmakelists))
        return data.strip()
    except:
        return None

def cmakelists_version():
    return parse_cmakelists(r'project\(.*VERSION\s+(\S*).*\)',
        lambda m: m.group(1))

def cmakelists_description():
    return parse_cmakelists(r'project\(.*DESCRIPTION\s+"([^"]*?)".*\)',
        lambda m: m.group(1))


class ChefConanFile(ConanFile):
    name = 'chef'
    version = cmakelists_version()
    description = cmakelists_description()
    url = 'https://github.com/Quincunx271/chef'
    no_copy_source = True
    generators = 'cmake'

    build_requires = (
        'Catch2/2.5.0@catchorg/stable',
    )
    exports_sources = 'cmake/*', 'src/*', 'CMakeLists.txt'

    def _configure_cmake(self):
        cmake = CMake(self)
        cmake.configure()
        return cmake

    def build(self):
        cmake = self._configure_cmake()
        cmake.build()

    def package(self):
        cmake = self._configure_cmake()
        cmake.install()
