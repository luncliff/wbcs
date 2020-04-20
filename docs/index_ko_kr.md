
# WBCS

Wide Byte Character Set subroutines  
는 사실 거짓말이고 C++ 개발자들이 SonarQube 사용법을 조금 더 쉽게 사용해 볼 수 있도록 하려고 만든 저장소

## Step 1

### 뭐...라고?

작성자는 학식(돈까스)를 먹으면서 [가르쳐줘! 코딩소녀](https://play.google.com/store/apps/details?id=com.lsm1987.CodingGirls&hl=ko)라는 아주 교육적인 게임(?)을 한 적이 있습니다.
이 게임을 통해서 SonarQube 라는 정적 분석 도구에 대해 알게 되었는데, 아쉬운 점들이 있지만 여러모로 작성자 개인에게 도움이 된 부분이 많았습니다.  
이 도구는 이름이 꽤 알려져 있음에도 불구하고, C++ 개발자가 처음 접했을때는 당황스러운 부분들이 좀 있습니다.
이 저장소에서는 SonarQube를 사용하기 위해 어떤 방법을 사용했는지 하나씩 짚어보는 형태로 다뤄보고자 합니다.

아래 뱃지는 이 프로젝트의 대시보드입니다.
결과적으로 여러분이 이런 뱃지를 사용하실 수 있게 된다면 좋겠습니다. 

[![Quality Gate Status](https://sonarcloud.io/api/project_badges/measure?project=luncliff-wbcs&metric=alert_status) ![Technical Debt](https://sonarcloud.io/api/project_badges/measure?project=luncliff-wbcs&metric=sqale_index)](https://sonarcloud.io/dashboard?id=luncliff-wbcs)

### SonarQube for C++

SonarQube의 홈페이지는 http://sonarqube.org 이며, 이를 사용중인 프로젝트들의 대시보드는 https://sonarcloud.io 에서 검색할 수 있습니다. 
안타깝게도 C++ 프로젝트는 거의 없지만 말이죠. (영업 중인건가?)
덕분에 당시에 메뉴얼 이외의 자료를 찾는 것이 꽤 고역이었습니다.

[C++ 프로젝트의 지원범위](https://www.sonarqube.org/features/multi-languages/cplus/)는 아래와 같습니다.
일반적인 C++만 사용하는 프로젝트 환경이라면 세세하게 신경쓸만한 부분은 없다고 할 수 있습니다.

* C++ 03 ~ 17
* Microsoft Windows, Linux, macOS
* Clang, MSVC 및 GCC 기반 컴파일러 들

언어 바인딩(파이썬, C#, JNI 등등...)을 지원하는 프로젝트라면 여러 언어가 혼용되고 있을텐데, 
작성자 개인의 경험으로는 언어 별로 빌드 설정을 정리하고, SonarQube를 적용하는 것이 여러모로 품이 덜 든다고 생각합니다.  
특히 여러 언어를 넘나드는 커버리지 분석을 한번에 진행할 수 없기 때문인 이유가 큽니다.

#### 그래서 무엇을 준비를 해야 하나요?

일반적으로 정적 분석은 빌드 결과물(object, 경고, 오류 메세지 등)을 사용하는데, SonarQube 역시 빌드 과정에서 출력되는 메세지를 수집하는 방법을 사용하고 있습니다.
그리고 이 수집된 결과물을 https://sonarcloud.io (혹은 직접 구동시킨 Docker 컨테이너)에 제출하는 것입니다.
이를 위해서는 2개의 프로그램을 설치해야 합니다.

* [수집(Build Wrapper)](https://docs.sonarqube.org/latest/analysis/languages/cfamily/)
* [분석(Scanner)](https://docs.sonarqube.org/latest/analysis/scan/sonarscanner/)

공개 프로젝트라면 [Travis CI와 같은 서비스에서 간단한 설정만으로 사용할 수 있도록 지원](https://docs.travis-ci.com/user/sonarcloud/)하기도 합니다.
[이 프로젝트는](https://github.com/luncliff/wbcs/blob/master/azure/steps-mac-sonarqube.yml) Azure Pipelines에서 지원되는 SonarQube 관련 Task들을 사용하고 있는데, **이해하고** 써야한다는 점만 빼면 단순한 편입니다.

### 프로젝트 구조 잡기

우선 정적 분석을 수행할 특정 플랫폼/컴파일러를 특정하는 것이 좋습니다.
물론 여러 플랫폼에 대한 분석을 수행할 수 있겠지만, 설정이 복잡해지면 이를 유지하는 것이 힘들어집니다.
이 문서에서는 Mac OS X 환경에서 Clang 컴파일러를 사용하는 것을 전제로, SonarQube를 사용하는 과정을 단계별로 스크립트로 작성해볼 것입니다.

이 프로젝트는 굉장히 작은 규모이기 때문에, 평범한 Makefile 프로젝트처럼 작성하되, CMake를 사용합니다.

```console
user@host:~/wbcs$ tree -L 2 .
.
├── sonar-project.properties
├── ...
├── azure
│   ├── ...
│   └── steps-mac-sonarqube.yml
├── externals
│   └── include
├── scripts
│   ├── ...
│   ├── run-sonar-wrapper.sh
│   └── run-sonarqube.sh
├── CMakeLists.txt
├── wbcs.cpp
├── wbcs.hpp
└── wbcs_test.cpp
```

프로젝트 구조는 개발자 개인의 취향, 혹은 정해진 지침을 따르는 경우가 있기 때문에, 각 파일들의 내용에 대해서만 적겠습니다.

* [sonar-project.properties](https://github.com/luncliff/wbcs/blob/master/sonar-project.properties)  
  SonarScanner가 사용할 설정을 정리한 파일입니다. `sonar.login`에 사용되는 Auth Token 이외의 내용이 작성되어 있습니다.

* [azure-pipelines.yml](https://github.com/luncliff/wbcs/blob/master/azure-pipelines.yml), [azure/](https://github.com/luncliff/wbcs/tree/master/azure)  
  **자동화는 [매우](https://namu.wiki/w/메우%20메우#s-7.1) 중요합니다**. CI 서비스들의 기능을 함께 사용하면 분석 결과를 누적시키기 쉽고, sonarcloud.io 에서 보다 유의미한 지표(시간에 따른 기술 부채의 변화)를 얻을 수 있습니다.

* [scripts/](https://github.com/luncliff/wbcs/tree/master/scripts)  
  build-wrapper만 다루는 경우 1개, sonar-scanner까지 다루는 경우 1개를 작성할 것입니다.

* [wbcs.cpp](https://github.com/luncliff/wbcs/blob/master/wbcs.cpp), [wbcs_test.cpp](https://github.com/luncliff/wbcs/blob/master/wbcs_test.cpp)  
  이 프로젝트는 구현 파일 1개, 테스트 파일 1개로 구성됩니다.

* [CMakeLists.txt](https://github.com/luncliff/wbcs/blob/master/CMakeLists.txt)  
  여러분이 어떤 프로젝트의 소스코드를 받았을 때, 그 안에 CMakeLists.txt가 있다면 사용방법은 Unix Makefiles와 크게 다르지 않습니다. (마땅히 그래야 합니다.)
  ```bash
  mkdir -p build && pushd build
      cmake ..          # ./configure
      cmake --build .   # make all
  popd
  ```

### [수집(Build Wrapper)](https://docs.sonarqube.org/latest/analysis/languages/cfamily/)

> TBA

### [분석(Scanner)](https://docs.sonarqube.org/latest/analysis/scan/sonarscanner/)

> TBA

## Step 2

> TBA: Step 1 을 기반으로, Coverage 분석을 사용하는 방법에 대해 다룹니다.

### gcov / lcov

여러분이 GCC를 사용하고 있다면 [gcov](https://gcc.gnu.org/onlinedocs/gcc/Gcov.html)를 사용해 커버리지를 획득할 수 있습니다. 
컴파일 옵션 `--coverage`를 사용하면, 테스트 프로그램을 빌드 했을 때 object 파일 외에도 `.gcno`가 함께 생성됩니다.
그리고 테스트 프로그램을 실행하면 각각에 해당하는 `.gcda`(gcov data)가 생성됩니다.
(실행할 때마다 횟수가 기록됩니다.)

다만 여기서 생성되는 파일들은 사람이 읽기 위한 것이 아니기 때문에, [lcov](https://wiki.documentfoundation.org/Development/Lcov)라는 프로그램을 사용해 HTML 페이지를 만들어 확인할 수 있습니다. 마치 JUnit 처럼 말이죠.


> 컴파일 옵션이기는 하지만 링커 옵션으로도 제공해야 합니다.

### lcov를 사용한 Coverage 시각화

Clang 컴파일러 역시 `--coverage` 옵션을 지원하며, gcov 파일들을 생성합니다.
다만 lcov는 설치가 필요합니다.

```
brew install lcov
```

이 저장소에서는 [유닉스 철학](https://ko.wikipedia.org/wiki/유닉스_철학)을 따라 정말 최소한의 내용만을 작성할 것입니다.  
필요하다면 메뉴얼을 읽어가며 살을 덧붙일 수 있을 것이라 생각합니다.

lcov를 사용하려면 알아야 하는 명령은 4개로 줄일 수 있습니다.

```bash
# create coverage info file
lcov --directory ${build_dir} --capture --output-file ${info_file}

# exclude files from coverage info
lcov --remove ${info_file} '*/externals/*'   --output-file ${info_file}
lcov --remove ${info_file} '*_test.cpp'      --output-file ${info_file}

# print file/coverage list
lcov --list ${info_file}
```

3개 뿐인데, 나머지 하나는 lcov가 아니라 genhtml 이라는 툴입니다. 이름 그대로, lcov 에서 생성한 파일에서 HTML 페이지를 만들어줍니다.

```bash
# coverage info >> HTML pages
mkdir -p ${save_dir}
genhtml ${info_file} -o ${save_dir}

open ${save_dir}/index.html
```

작성자는 빌드 직후 lcov 분석을 수행하는 것을 상정하고  [스크립트](https://github.com/luncliff/wbcs/blob/master/scripts/run-lcov.sh)에는 절차를 설명하고 있습니다.

```bash
brew install lcov

mkdir build && pushd build
    cmake .. -DCMAKE_CXX_FLAGS="--coverage -g -O0" -DBUILD_TESTING=true
    cmake --build .
    ctest
popd
bash ./scripts/run-lcov.sh build   # <--- this file
open ./docs/coverage/index.html
```

### .gcov 만들기

https://coveralls.io 와 같은 서비스들은 대부분 .gcov 파일을 요구합니다. 
사실 .gcda 파일을 만드는 부분까지 진행되어 있다면, 즉 테스트 프로그램이 실행까지 완료하였다면, [이를 수집하여 .gcov 파일들을 만드는 것](https://github.com/luncliff/wbcs/blob/master/scripts/run-gcov.sh)은 굉장히 쉽습니다.

gcov 에세 gcda 파일을 입력으로 주기만 하면 되기 때문이죠.

```bash
pushd ${build_dir}
    for fpath in $(find $(pwd) -name *.gcda)
    do
        gcov -abcf ${fpath}
    done
popd
```

> TBA: CMake 프로젝트에서의 불편한 점

> TBA

## Q&A

### 그런데 Codacy, LGTM을 두고 굳이 SonarQube를 쓸 필요가 있나요?

Azure Pipelines에는 VS Enterprise가 설치되어 있어서 커버리지 분석이 지원됩니다. 그런데 이 파일을 지원하는게 SonarQube와 CppDepend 뿐인 것 같습니다.

