BINDIR = '/usr/local/bin'
BLOCK_MESSAGE_KEYS = []
BUILD_TYPE = 'app'
BUNDLE_NAME = 'BatteryHelper.pbw'
DEFINES = ['RELEASE']
LIBDIR = '/usr/local/lib'
LIB_DIR = 'node_modules'
LIB_JSON = [{u'license': u'MIT', u'name': u'pebble-clay', u'repository': {u'url': u'git+https://github.com/pebble/clay.git', u'type': u'git'}, u'author': u'Pebble Technology', u'bugs': {u'url': u'https://github.com/pebble/clay/issues'}, u'version': u'1.0.4', 'js_paths': ['node_modules/pebble-clay/dist/js/index.js'], u'dependencies': {}, u'scripts': {u'pebble-publish': u'npm run pebble-clean && npm run build && pebble build && pebble package publish && npm run pebble-clean', u'test-travis': u'./node_modules/.bin/gulp && ./node_modules/.bin/karma start ./test/karma.conf.js --single-run --browsers chromeTravisCI && ./node_modules/.bin/eslint ./', u'pebble-build': u'npm run build && pebble build', u'pebble-clean': u'rm -rf tmp src/js/index.js && pebble clean', u'lint': u'./node_modules/.bin/eslint ./', u'dev': u'gulp dev', u'build': u'gulp', u'test': u'./node_modules/.bin/gulp && ./node_modules/.bin/karma start ./test/karma.conf.js --single-run', u'test-debug': u'(export DEBUG=true && ./node_modules/.bin/gulp && ./node_modules/.bin/karma start ./test/karma.conf.js --no-single-run)'}, u'keywords': [u'pebble', u'config', u'configuration', u'pebble-package'], u'devDependencies': {u'chai': u'^3.4.1', u'mocha': u'^2.3.4', u'through': u'^2.3.8', u'gulp-inline': u'0.0.15', u'karma-source-map-support': u'^1.1.0', u'deepcopy': u'^0.6.1', u'eslint-plugin-standard': u'^1.3.1', u'stringify': u'^3.2.0', u'gulp-insert': u'^0.5.0', u'gulp': u'^3.9.0', u'gulp-htmlmin': u'^1.3.0', u'deamdify': u'^0.2.0', u'bourbon': u'^4.2.6', u'eslint-config-pebble': u'^1.2.0', u'eslint': u'^1.5.1', u'karma-coverage': u'^0.5.3', u'watchify': u'^3.7.0', u'require-from-string': u'^1.1.0', u'gulp-sourcemaps': u'^1.6.0', u'karma-mocha': u'^0.2.1', u'sinon': u'^1.17.3', u'joi': u'^6.10.1', u'browserify': u'^13.0.0', u'sassify': u'^0.9.1', u'gulp-autoprefixer': u'^3.1.0', u'karma-mocha-reporter': u'^1.1.5', u'autoprefixer': u'^6.3.1', u'browserify-istanbul': u'^0.2.1', u'karma-threshold-reporter': u'^0.1.15', u'gulp-sass': u'^2.1.1', u'vinyl-source-stream': u'^1.1.0', u'gulp-uglify': u'^1.5.2', u'karma-chrome-launcher': u'^0.2.2', u'vinyl-buffer': u'^1.0.0', u'del': u'^2.0.2', u'karma': u'^0.13.19', u'karma-browserify': u'^5.0.1', u'tosource': u'^1.0.0', u'postcss': u'^5.0.14'}, u'pebble': {u'sdkVersion': u'3', u'capabilities': [u'configurable'], u'projectType': u'package', u'targetPlatforms': [u'aplite', u'basalt', u'chalk', u'diorite', u'emery'], u'resources': {u'media': []}}, 'path': 'node_modules/pebble-clay/dist', u'homepage': u'https://github.com/pebble/clay#readme', u'description': u'Pebble Config Framework'}]
LIB_RESOURCES_JSON = {u'pebble-clay': []}
MESSAGE_KEYS = {u'fixedTime': 10003, u'sendWhenAppOpened': 10002, u'endpoint': 10004, u'requestSendToEndpoint': 10006, u'JSReady': 10005, u'sendWhenBatteryChanged': 10001, u'sendAtFixedTime': 10000}
MESSAGE_KEYS_DEFINITION = '/home/rebble/Documents/BatteryHelper/BatteryHelper/build/src/message_keys.auto.c'
MESSAGE_KEYS_HEADER = '/home/rebble/Documents/BatteryHelper/BatteryHelper/build/include/message_keys.auto.h'
MESSAGE_KEYS_JSON = '/home/rebble/Documents/BatteryHelper/BatteryHelper/build/js/message_keys.json'
NODE_PATH = '/home/rebble/.pebble-sdk/SDKs/current/node_modules'
PEBBLE_SDK_COMMON = '/home/rebble/.pebble-sdk/SDKs/current/sdk-core/pebble/common'
PEBBLE_SDK_ROOT = '/home/rebble/.pebble-sdk/SDKs/current/sdk-core/pebble'
PREFIX = '/usr/local'
PROJECT_INFO = {'appKeys': {u'fixedTime': 10003, u'sendWhenAppOpened': 10002, u'endpoint': 10004, u'requestSendToEndpoint': 10006, u'JSReady': 10005, u'sendWhenBatteryChanged': 10001, u'sendAtFixedTime': 10000}, u'sdkVersion': u'3', u'displayName': u'Battery Helper', u'uuid': u'bfa0936b-a7ab-433c-8092-a93e934b9944', u'messageKeys': {u'fixedTime': 10003, u'sendWhenAppOpened': 10002, u'endpoint': 10004, u'requestSendToEndpoint': 10006, u'JSReady': 10005, u'sendWhenBatteryChanged': 10001, u'sendAtFixedTime': 10000}, 'companyName': u'Stefan Bauwens', u'enableMultiJS': True, u'targetPlatforms': [u'aplite', u'basalt', u'chalk', u'diorite'], u'capabilities': [u'configurable'], 'versionLabel': u'1.0', 'longName': u'Battery Helper', 'shortName': u'Battery Helper', u'watchapp': {u'watchface': False}, u'resources': {u'media': []}, 'name': u'BatteryHelper'}
REQUESTED_PLATFORMS = [u'aplite', u'basalt', u'chalk', u'diorite']
RESOURCES_JSON = []
SANDBOX = False
SUPPORTED_PLATFORMS = ['basalt', 'aplite', 'emery', 'diorite', 'chalk']
TARGET_PLATFORMS = ['diorite', 'chalk', 'basalt', 'aplite']
TIMESTAMP = 1741385426
USE_GROUPS = True
VERBOSE = 0
WEBPACK = '/home/rebble/.pebble-sdk/SDKs/current/node_modules/.bin/webpack'
