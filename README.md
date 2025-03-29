# LDOPA - C++ library for process mining

`LDOPA` - это библиотека для решения задач process mining  на языке C++, предоставляющая основные модели и алгоритмы, такие как:

- Журнал событий.

- Система переходов и алгоритмы её синтеза.

- Сеть Петри и алгоритмы её синтеза.

- Подсчет метрик моделей.

- Алгоритм частотной редукции систем переходов.

- Алгоритм сворачивания циклов систем переходов.




## Работа с библиотекой

Библиотека обернута в пространство имён `xi::ldopa` и содержит в себе:

- Пространство `eventlog` c концептом журнала событий и типами `CSVLog` и `SQLiteLog`, являющимися реализациями для `.CSV` и `.SQ3` таблиц соответственно.

- Типы `BoostGraphP` и `BoostBidiGraphP`: обертки над `Boost Graph Library`, реализующие функци направленного и двунаправленного графа соотвественно.

- Пространство `ts` c концептами систем переходов и алгоритмами синтеза и редукции. 

- Пространство `pn` с концептами сетей Петри и алгоритмами синтеза.

- Типы `ElapsedTimeStore` и `ProgressCallback` для отслеживания времени работы алгоритма и его прогресса соответственно.

Примеры работы с библиотекой можно найти в [тестах](test) в папке `test`.

## Установка библиотеки

Для установки потребуется `CMake`, начиная с версии `3.15`. Более подробно об установке можно прочитать [здесь](BUILDING.md). Пример установки через `CMake`:
```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release
cmake --build build
cmake --install build
```

## Подключение библиотеки


Исходный код библиотеки основан на файлах формата `.h`, находящихся в папках внутри `include`, и файлах формата `.cpp`, находящихся в папках внутри `source`, потому для использования библиотеки потребуется при подключении конкретных файлов указывать явный путь, например:

```cpp
#include <xi-0.1.0/xi/ldopa/ts/models/evlog_ts_fold.h>
#include <xi-0.1.0/xi/ldopa/ts/algos/ts_folding_builder.h>
#include <xi-0.1.0/xi/ldopa/ts/algos/cycle_condenser.h>
#include <xi-0.1.0/xi/ldopa/eventlog/sqlite/sqlitelog.h>


using namespace xi::ldopa;

std::string path;
eventlog::SQLiteLog log(path);

ts::AttrListStateIDsPool pool;

ts::PrefixStateFunc fnc(&log, &pool);

ts::TsFoldBuilder bldr(&log, &fnc, &pool);

ts::EvLogTSWithParVecs* ts = bldr.build(); 
```

Для сборки внешнего проекта рекомендуется использовать Cmake, вместе с библиотекой подключив `boost`. Пример файла `CMakeLists.txt` для внешнего проекта:
```
cmake_minimum_required(VERSION 3.15)

project("project" VERSION 0.9 DESCRIPTION "A project with external library")

find_package(Boost REQUIRED REQUIRED graph)
find_package(xi REQUIRED)

add_executable(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
    PRIVATE
        main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE xi::xi)
```

## Тестирование

Для корректной работы библиотеки реализованы тесты в папке `test`. Для тестовой сборки потребуется `CMake`, начиная с версии `3.15`. Определён таргет `xi_test` для запуска всех тестов. Более подробно о тестировании можно прочитать [здесь](HACKING.md). Пример сборки тестов через `CMake`:
```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```

## О проекте

Данная библиотека является курсовым проектом в рамках НИУ ВШЭ на образовательной программе "Прикладная математика и информатика", 2025 год. Работу выполнил Хамзин Рамиль, студент 2 курса.