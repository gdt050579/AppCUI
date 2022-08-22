import sys
import os

if len(sys.argv) < 2:
    print("Failed to obtain AppCUI.hpp location")
    exit(1)

header_location = sys.argv[1]
if not os.path.exists(header_location):
    print("Path {} does not exists!".format(header_location))
    exit(1)

found_version = False
with open(header_location, 'r') as f:
    for line in f:
        if line.startswith('#define APPCUI_VERSION '):
            version = line.split('#define APPCUI_VERSION ')[
                1].strip(' \r\n\t\"')
            found_version = True
            print(version)
            os.putenv('APPCUI_VERSION', version)
            break

if not found_version:
    print("Failed to find APPCUI_VERSION")
    exit(1)

exit(0)
