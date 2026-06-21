import os, sys, distro, math
lib_path = os.path.abspath(os.path.join(__file__,'..','..','src'))
sys.path.append(lib_path)
from geofix import DfAnalyzer

dfAnalyzer = DfAnalyzer(enable_logging = True)

result = dfAnalyzer.add_bearings(
    site_coordinates = [2.350128676656633, 48.86129501083692, 13.409665746048544, 52.521323096219966, 13.779348093551762, 45.64638482429674],
    bearings = [90, 225, 315],
    bearing_errors = [2, 2, 2]
)

print(result)
print('COMPILE TIME: ', dfAnalyzer.get_compile_time())
print('VERSION     : ', dfAnalyzer.get_native_version())
print('COMMIT HASH : ', dfAnalyzer.get_commit_hash())
