import sys, os, distro
 
if sys.version_info[0] == 2:
    print('Python 2 is not supported')
    sys.exit(0)

if sys.version_info[0] == 3:
    if os.name == 'posix':
        if distro.name().startswith('Debian'):
            from .native import pygeo_debian12 as pygeo
        elif distro.name().startswith('CentOS'):
            from .native import pygeo_centos7 as pygeo
        elif distro.name().startswith('Rocky'):
            try:
                from .native import pygeo_rocky8 as pygeo
            except:
                from .native import pygeo_rocky9 as pygeo
        elif distro.name().startswith('Fedora'):
            try:
                from .native import pygeo_fedora40 as pygeo
            except:
                try:
                    from .native import pygeo_fedora41 as pygeo
                except:
                    try:
                        from .native import pygeo_fedora42 as pygeo
                    except:
                        try:
                            from .native import pygeo_fedora43 as pygeo
                        except:
                            print('Problem with Fedora 40, 41, 42 and 43')
        else:
            print(distro.linux_distribution()[0] + ' is not supported')
            sys.exit(0)
    if os.name == 'nt':
        from .native import pygeo_windows as pygeo
        # print('windows is not supported')
        # sys.exit(0)

class DfAnalyzer:

    def __init__(self, enable_logging = False):
        self.analyzer = pygeo.Analyzer(enable_logging)

    def get_compile_time(self):
        """get compilation time for the native library"""
        return self.analyzer.getCompileTime()

    def get_native_version(self):
        """get compilation time for the native library"""
        return self.analyzer.getVersion()

    def get_commit_hash(self):
        """get compilation time for the native library"""
        return self.analyzer.getCommitHash()

    def add_bearings(self, site_coordinates, bearings, bearing_errors):
        """add bearings and return fix result to the df analyzer"""
        if not (len(site_coordinates)/2 == len(bearings) == len(bearing_errors)):
            raise Exception("The length of bearings, bearing_errors and site_coordinate pairs must be equal")

        bearings.extend(bearing_errors)
        self.analyzer.loadDirections(int(len(bearings)/2), site_coordinates, bearings)
        result = self.analyzer.getFixEllipse(20, 0.95)
        if result['status'] < 0:
            result['center'] = ()
            result['ellipse'] = ()
        return result

    def clear(self):
        return self.analyzer.clear()

