#
# DANTE :: System Automatycznych Testów Jednostkowych oraz Akceptacyjnych
# Tomasz Jaworski, 2018-2020
#
# Plik wygenerowany automatycznie
# Znacznik czasowy: 2023-01-25 18:29:40.336265
#

cmake_minimum_required(VERSION 3.17)
project(project1 C)

# Przyjmij standard C11
set(CMAKE_C_STANDARD 11)

# Ustaw opcje kompilatora (z raportu Dante)
add_compile_options(

)

# Ustaw opcje konsolidatora (z raportu Dante)
add_link_options(
)

# Utwórz target 'project1' i dodaj do niego pliki źródlowe:
add_executable(project1
)

# Dołącz niezbędne biblioteki
target_link_libraries(project1
)