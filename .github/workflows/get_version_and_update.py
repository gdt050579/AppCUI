import sys
import os
import shutil

if len(sys.argv) < 2:
    print("Failed to obtain AppCUI.hpp location")
    exit(1)

header_location = sys.argv[1]
if not os.path.exists(header_location):
    print("Path {} does not exists!".format(header_location))
    exit(1)

default_version_to_update = 1  # major=0, minor=1, patch=2
if len(sys.argv) > 2:
    version_to_update = sys.argv[2]
    defined_versions = {
        "major": 0,
        "minor": 1,
        "patch": 2
    }
    default_version_to_update = defined_versions[version_to_update]

reset_lower_versions = True

header_output_location = header_location+'.new'
found_version = False
with open(header_location, 'r') as f:
    with open(header_output_location, 'w') as g:
        for line in f:
            if line.startswith('#define APPCUI_VERSION '):
                version = line.split('#define APPCUI_VERSION ')[
                    1].strip(' \r\n\t\"')

                version_array = version.split('.')

                value = int(version_array[default_version_to_update])+1
                version_array[default_version_to_update] = value

                for i in range(default_version_to_update+1, 3):
                    version_array[i] = 0

                version = "{}.{}.{}".format(
                    version_array[0], version_array[1], version_array[2])
                line = '#define APPCUI_VERSION "{}"\n'.format(version)
                found_version = True
                os.putenv('APPCUI_VERSION', version)
                print(version)
                break
            g.write(line)

if not found_version:
    print("Failed to find APPCUI_VERSION")
    exit(1)

shutil.move(header_output_location, header_location)
exit(0)
