# python3
import shutil
from zipfile import ZipFile
import os
import sys

allowed_extensions = {
    'windows': {
        'shared': ['dll', 'pdb'],
        'static': ['lib', 'pdb'],
    },
    'linux': {
        'shared': ['so'],
        'static': ['a'],
    },
    'apple': {
        'shared': ['dylib'],
        'static': ['a'],
    }
}

allowed_release_objects = ['libAppCUI', 'AppCUI.hpp']  # TODO: could be improved with regex


def process_package(configuration, src_path, binaries_path):
    # print(os.listdir(src_path))
    include_path = os.path.join(src_path, 'AppCUI', 'include', 'AppCUI.hpp')
    bin_configurations = os.listdir(binaries_path)
    for bin_config in bin_configurations:
        bin_config_location = os.path.join(binaries_path, bin_config)
        bin_include_folder = os.path.join(bin_config_location, 'include')

        if not os.path.exists(bin_include_folder):
            os.mkdir(bin_include_folder)
        shutil.copy(include_path, bin_include_folder)

        zip_location = os.path.join(binaries_path, 'AppCUI_' + configuration + '_' + bin_config + '.zip')
        with ZipFile(zip_location, 'w') as zipObj:
            for folderName, subfolders, filenames in os.walk(bin_config_location):
                rel_path = os.path.relpath(folderName, bin_config_location)
                for filename in filenames:
                    for allowed_release in allowed_release_objects:
                        if str(filename).startswith(allowed_release):
                            filePath = os.path.join(folderName, filename)
                            zipObj.write(filePath, os.path.join(rel_path, os.path.basename(filePath)))
                            break


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Failed to get params: {} <configuration> <src_path> <binaries_path>'.format(sys.argv[0]))
        exit(1)
    process_package(sys.argv[1], sys.argv[2], sys.argv[3])
