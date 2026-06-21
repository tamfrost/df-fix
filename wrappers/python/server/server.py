from flask import Flask, request, jsonify, render_template
import os, sys

lib_path = os.path.abspath(os.path.abspath(os.path.join(__file__,'..','..','package','src')))
sys.path.append(lib_path)

from geofix import DfAnalyzer

dfAnalyzer = DfAnalyzer(enable_logging = False)

print('COMPILE TIME: ', dfAnalyzer.get_compile_time())
print('VERSION     : ', dfAnalyzer.get_native_version())
print('COMMIT HASH : ', dfAnalyzer.get_commit_hash())

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/api/v1/ping', methods=['GET'])
def getPing():
    return jsonify({"reply": "pong"}) 

@app.route('/api/v1/getFixPoint', methods=['POST'])
def getFixPoins():
    fixParameters = request.get_json()
    result = dfAnalyzer.add_bearings(
        site_coordinates = fixParameters['siteCoordinates'],
        bearings = fixParameters['bearings'],
        bearing_errors = fixParameters['bearingErrors']
    )
    buildInfo = {
        'compileTime': dfAnalyzer.get_compile_time(),
        'version'     : dfAnalyzer.get_native_version(),
        'commitHash' : dfAnalyzer.get_commit_hash()
    }
    result['buildInfo'] = buildInfo
    return jsonify(result) 


if __name__ == "__main__":
    port = int(os.environ.get('FLASK_PORT', 8181))
    app.run(debug=True, host='0.0.0.0', port=port)