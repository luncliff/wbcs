#!/bin/bash
#
# References
#   - https://docs.sonarqube.org/latest/analysis/languages/cfamily/
#   - https://docs.sonarqube.org/latest/analysis/scan/sonarscanner/
#   - https://docs.travis-ci.com/user/sonarcloud/
#
sonar_key=${1:-"-----"}
build_dir=${2:-"build"}

if [ ${sonar_key} = "-----" ]; then
  echo "sonar_key must be specified !!!"
  echo ""
  echo "run-sonarqube.sh \${sonar_login_key}"
  echo ""
  exit 1;
fi

# Download and unzip SonarQube Tools
wrapper="build-wrapper-macosx-x86"
zip_path="${wrapper}.zip"
if [[ -f ${zip_path} ]]; then
  unzip -qq -o ${zip_path}
else
  wget -q https://sonarcloud.io/static/cpp/${wrapper}.zip
  unzip -qq -o ${zip_path}
fi

scanner="sonar-scanner"
zip_path="${scanner}-cli-4.2.0.1873-macosx.zip"
if [[ -f ${zip_path} ]]; then
  unzip -qq -o ${zip_path}
else
  wget https://binaries.sonarsource.com/Distribution/sonar-scanner-cli/${zip_path}
  unzip -qq -o ${zip_path}
fi

# sonar scanner cli has different name for unzipped directory
export PATH="${PATH}:$(pwd)/${scanner}-4.2.0.1873-macosx/bin"
echo "using scanner: $(which ${scanner})"

export PATH="${PATH}:$(pwd)/${wrapper}"
echo "using wrapper: $(which ${wrapper})"

${wrapper} --version
${wrapper} --out-dir bw-output \
  cmake --build ${build_dir} --clean-first

# Report with the properties
# ${scanner} --version
${scanner} -Dproject.settings=sonar-project.properties \
           -Dsonar.login=${sonar_key}
