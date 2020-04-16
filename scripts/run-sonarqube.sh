#!/bin/bash
#
# References
#   - https://docs.sonarqube.org/latest/analysis/languages/cfamily/
#   - https://docs.sonarqube.org/latest/analysis/scan/sonarscanner/
#   - https://docs.travis-ci.com/user/sonarcloud/
#
wrapper=${1:-"build-wrapper-macosx-x86"}
build_dir=${2:-"build"}

# Download and unzip SonarQube Tools
zip_path="${wrapper}.zip"
if [[ -f ${zip_path} ]]; then
  unzip -o ${zip_path}
else
  wget -q https://sonarcloud.io/static/cpp/${wrapper}.zip
  unzip ${zip_path}
fi

scanner="sonar-scanner-cli-4.2.0.1873-macosx"
zip_path="${scanner}.zip"
if [[ -f ${zip_path} ]]; then
  unzip -o ${zip_path}
else
  wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/${scanner}.zip
  unzip -q -o ${scanner}.zip
fi
export PATH="${PATH}:$(pwd)/${scanner}/bin"
echo ""
echo "using scanner: $(which ${scanner})"
echo ""

# Acquire absolute path of the build directory
build_path=$(find $(pwd) -name ${build_dir})
echo ""
echo "using build dir: ${build_path}"
echo ""

export PATH="${PATH}:$(pwd)/${wrapper}"
echo ""
echo "using wrapper: $(which ${wrapper})"
echo ""

${wrapper} --version
${wrapper} --out-dir bw-output \
  cmake --build ${build_dir} --clean-first

# Report with the properties
# sonar-scanner -Dproject.settings=sonar-project.properties \
#               -Dsonar.host.url=https://sonarcloud.io:9000
sonar-scanner
