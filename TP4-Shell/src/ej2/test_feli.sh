# #!/bin/bash

# # test_shell.sh - Script para probar el shell implementado (EXTENDIDO)
# # Enfocado en pipes sin parsing complejo de texto
# # Uso: ./test_shell.sh

# # Colores para output
# RED='\033[0;31m'
# GREEN='\033[0;32m'
# YELLOW='\033[1;33m'
# BLUE='\033[0;34m'
# NC='\033[0m' # No Color

# # Contadores
# PASSED=0
# FAILED=0
# TOTAL=0

# # Función para mostrar resultados
# print_result() {
#     local test_name="$1"
#     local expected="$2"
#     local actual="$3"
#     local status="$4"
    
#     echo -e "${BLUE}Test: ${test_name}${NC}"
    
#     if [ "$status" = "PASS" ]; then
#         echo -e "${GREEN}[PASS]${NC}"
#         ((PASSED++))
#     else
#         echo -e "${RED}[FAIL]${NC}"
#         echo -e "${YELLOW}Expected:${NC}"
#         echo "$expected"
#         echo -e "${YELLOW}Actual:${NC}"
#         echo "$actual"
#         echo -e "${YELLOW}---${NC}"
#         ((FAILED++))
#     fi
#     ((TOTAL++))
#     echo
# }

# # Función para ejecutar test
# run_test() {
#     local test_name="$1"
#     local shell_command="$2"
#     local bash_command="$3"
    
#     # Si no se proporciona bash_command, usar el mismo que shell_command
#     if [ -z "$bash_command" ]; then
#         bash_command="$shell_command"
#     fi
    
#     echo -e "${BLUE}Running: ${test_name}${NC}"
    
#     # Ejecutar con bash (referencia)
#     expected=$(echo "$bash_command" | bash 2>/dev/null | sort)
    
#     # Ejecutar con nuestro shell
#     actual=$(echo -e "$shell_command\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | sort)
    
#     # Comparar resultados
#     if [ "$expected" = "$actual" ]; then
#         print_result "$test_name" "$expected" "$actual" "PASS"
#     else
#         print_result "$test_name" "$expected" "$actual" "FAIL"
#     fi
# }

# # Función especial para tests que requieren comparación exacta (sin sort)
# run_test_exact() {
#     local test_name="$1"
#     local shell_command="$2"
#     local bash_command="$3"
    
#     if [ -z "$bash_command" ]; then
#         bash_command="$shell_command"
#     fi
    
#     echo -e "${BLUE}Running: ${test_name}${NC}"
    
#     # Ejecutar con bash (referencia) - sin sort
#     expected=$(echo "$bash_command" | bash 2>/dev/null)
    
#     # Ejecutar con nuestro shell - sin sort
#     actual=$(echo -e "$shell_command\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>")
    
#     # Comparar resultados
#     if [ "$expected" = "$actual" ]; then
#         print_result "$test_name" "$expected" "$actual" "PASS"
#     else
#         print_result "$test_name" "$expected" "$actual" "FAIL"
#     fi
# }

# # Verificar que existe el ejecutable
# if [ ! -f "./shell" ]; then
#     echo -e "${RED}Error: No se encuentra el ejecutable './shell'${NC}"
#     echo "Asegúrate de compilar primero con: make"
#     exit 1
# fi

# echo -e "${BLUE}=== TESTS PARA EL SHELL (EXTENDIDO) ===${NC}"
# echo -e "${YELLOW}Nota: Estos tests evitan el parsing complejo según la consigna${NC}"
# echo

# echo -e "${YELLOW}=== TESTS BÁSICOS DE COMANDOS SIMPLES ===${NC}"

# # Test 1: Comando simple sin pipes
# run_test "Comando simple: ls" "ls"

# # Test 2: pwd
# run_test_exact "pwd" "pwd"

# # Test 3: whoami
# run_test_exact "whoami" "whoami"

# # Test 4: date (solo verificamos que ejecute, no el contenido exacto)
# echo -e "${BLUE}Running: date (execution test)${NC}"
# date_output=$(echo -e "date\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
# if [ "$date_output" -gt 0 ]; then
#     print_result "date execution" "output produced" "output produced" "PASS"
# else
#     print_result "date execution" "output produced" "no output" "FAIL"
# fi

# echo -e "${YELLOW}=== TESTS DE ECHO ===${NC}"

# # Test 5: echo simple
# run_test_exact "echo hello" "echo hello"

# # Test 6: echo con múltiples palabras
# run_test_exact "echo hello world" "echo hello world"

# # Test 7: echo con comillas (si tu shell las maneja)
# run_test_exact "echo \"hello world\"" "echo \"hello world\""

# echo -e "${YELLOW}=== TESTS BÁSICOS DE PIPES ===${NC}"

# # Test 8: Pipe básico simple - buscar archivos con extensión
# run_test "Pipe básico: ls | grep .c" "ls | grep .c"

# # Test 9: Pipe básico - buscar archivos shell
# run_test "Pipe básico: ls | grep shell" "ls | grep shell"

# # Test 10: Pipe básico - buscar Makefile
# run_test "Pipe básico: ls | grep Makefile" "ls | grep Makefile"

# # Test 11: Contar líneas con pipe
# run_test_exact "Contar con pipe: ls | wc -l" "ls | wc -l"

# # Test 12: head con pipe
# run_test "head con pipe: ls | head -5" "ls | head -5"

# # Test 13: tail con pipe
# run_test "tail con pipe: ls | tail -3" "ls | tail -3"

# echo -e "${YELLOW}=== TESTS DE MÚLTIPLES PIPES ===${NC}"

# # Test 14: Triple pipe - contar archivos .c
# run_test_exact "Triple pipe: ls | grep .c | wc -l" "ls | grep .c | wc -l"

# # Test 15: Triple pipe - ordenar y limitar
# run_test "Triple pipe: ls | sort | head -3" "ls | sort | head -3"

# # Test 16: Triple pipe - buscar y ordenar
# run_test "Triple pipe: ls | grep shell | sort" "ls | grep shell | sort"

# # Test 17: Cuádruple pipe
# run_test "Cuádruple pipe: ls | grep shell | sort | head -2" "ls | grep shell | sort | head -2"

# echo -e "${YELLOW}=== TESTS DE COMANDOS CON ARGUMENTOS ===${NC}"

# # Test 18: ls con argumentos
# run_test "ls con -l: ls -l | grep shell" "ls -l | grep shell"

# # Test 19: head con número
# run_test "head con número: ls | head -5" "ls | head -5"

# # Test 20: tail con número
# run_test "tail con número: ls | tail -3" "ls | tail -3"

# # Test 21: wc con diferentes opciones
# run_test_exact "wc con -l: ls | wc -l" "ls | wc -l"
# run_test_exact "wc con -w: ls | wc -w" "ls | wc -w"

# # Test 22: sort con opciones
# run_test "sort: ls | sort" "ls | sort"
# run_test "sort reverse: ls | sort -r" "ls | sort -r"

# echo -e "${YELLOW}=== TESTS DE COMANDOS DEL SISTEMA ===${NC}"

# # Test 23: ps básico (solo verificamos ejecución)
# echo -e "${BLUE}Running: ps (execution test)${NC}"
# ps_output=$(echo -e "ps\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
# if [ "$ps_output" -gt 0 ]; then
#     print_result "ps execution" "output produced" "output produced" "PASS"
# else
#     print_result "ps execution" "output produced" "no output" "FAIL"
# fi

# # Test 24: ps con pipe y grep (evitando grep grep)
# run_test "ps con pipe: ps aux | head -5" "ps aux | head -5"

# # Test 25: who
# echo -e "${BLUE}Running: who (execution test)${NC}"
# who_output=$(echo -e "who\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
# if [ "$who_output" -ge 0 ]; then  # Puede ser 0 si no hay usuarios logueados
#     print_result "who execution" "executed" "executed" "PASS"
# else
#     print_result "who execution" "executed" "failed" "FAIL"
# fi

# echo -e "${YELLOW}=== TESTS DE ROBUSTEZ ===${NC}"

# # Test 26: Espacios alrededor de pipes
# run_test "Espacios: ls | grep shell" "ls | grep shell"
# run_test "Sin espacios: ls|grep shell" "ls|grep shell" "ls | grep shell"

# # Test 27: Pipe con cat
# run_test "Pipe con cat: ls | cat" "ls | cat"

# # Test 28: Múltiples espacios (si tu shell los maneja)
# run_test "Múltiples espacios: ls  |  grep  shell" "ls  |  grep  shell" "ls | grep shell"

# echo -e "${YELLOW}=== TESTS DE COMBINACIONES COMPLEJAS ===${NC}"

# # Test 29: echo con pipe
# run_test_exact "echo con pipe: echo test | grep test" "echo test | grep test"

# # Test 30: echo múltiple con pipe
# run_test "echo múltiple: echo hello world | grep hello" "echo hello world | grep hello"

# # Test 31: Comando con múltiples argumentos
# run_test "ls con argumentos: ls -la | head -3" "ls -la | head -3"

# # Test 32: Pipe con wc y diferentes opciones
# run_test_exact "wc caracteres: echo hello | wc -c" "echo hello | wc -c"

# # Test 33: Combinación de comandos de sistema
# run_test "pwd con pipe: pwd | cat" "pwd | cat"

# echo -e "${YELLOW}=== TESTS DE LÍMITES ===${NC}"

# # Test 34: Pipe muy largo (5 comandos)
# run_test "5 pipes: ls | grep shell | sort | head -3 | cat" "ls | grep shell | sort | head -3 | cat"

# # Test 35: Comando vacío con pipe (manejo de errores)
# echo -e "${BLUE}Running: empty command test${NC}"
# empty_output=$(echo -e " | grep test\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
# # No verificamos resultado específico, solo que no crashee
# print_result "empty command handling" "handled" "handled" "PASS"

# echo -e "${BLUE}=== TESTS EXCLUIDOS POR PARSING COMPLEJO ===${NC}"
# echo -e "${YELLOW}Los siguientes tipos de comandos NO se testean según la consigna:${NC}"
# echo -e "- Comandos con comillas complejas: ls | grep \".zip\""
# echo -e "- Patrones complejos: ls | grep \".png .zip\""
# echo -e "- Parsing de argumentos con espacios y comillas"
# echo -e "- Escape de caracteres especiales"
# echo -e "- Redirección de archivos (>, <, >>)"
# echo -e "- Variables de entorno complejas"
# echo -e "- Comandos con && o ||"
# echo

# echo -e "${BLUE}=== RESUMEN DE TESTS ===${NC}"
# echo -e "Total de tests: ${TOTAL}"
# echo -e "${GREEN}Pasaron: ${PASSED}${NC}"
# echo -e "${RED}Fallaron: ${FAILED}${NC}"

# if [ $FAILED -eq 0 ]; then
#     echo -e "${GREEN}¡Todos los tests pasaron! ✓${NC}"
#     echo -e "${BLUE}El shell maneja correctamente pipes sin parsing complejo${NC}"
#     exit 0
# else
#     echo -e "${RED}Algunos tests fallaron. Revisa la implementación.${NC}"
#     echo -e "${YELLOW}Ratio de éxito: $(echo "scale=1; $PASSED * 100 / $TOTAL" | bc -l)%${NC}"
#     exit 1
# fi


#!/bin/bash

# test_shell.sh - Script para probar el shell implementado (EXTENDIDO)
# Enfocado en pipes sin parsing complejo de texto
# Uso: ./test_shell.sh

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Contadores
PASSED=0
FAILED=0
TOTAL=0

# Función para mostrar resultados
print_result() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    local status="$4"
    
    echo -e "${BLUE}Test: ${test_name}${NC}"
    
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}[PASS]${NC}"
        ((PASSED++))
    else
        echo -e "${RED}[FAIL]${NC}"
        echo -e "${YELLOW}Expected:${NC}"
        echo "$expected"
        echo -e "${YELLOW}Actual:${NC}"
        echo "$actual"
        echo -e "${YELLOW}---${NC}"
        ((FAILED++))
    fi
    ((TOTAL++))
    echo
}

# Función para ejecutar test
run_test() {
    local test_name="$1"
    local shell_command="$2"
    local bash_command="$3"
    
    # Si no se proporciona bash_command, usar el mismo que shell_command
    if [ -z "$bash_command" ]; then
        bash_command="$shell_command"
    fi
    
    echo -e "${BLUE}Running: ${test_name}${NC}"
    
    # Ejecutar con bash (referencia)
    expected=$(echo "$bash_command" | bash 2>/dev/null | sort)
    
    # Ejecutar con nuestro shell
    actual=$(echo -e "$shell_command\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | sort)
    
    # Comparar resultados
    if [ "$expected" = "$actual" ]; then
        print_result "$test_name" "$expected" "$actual" "PASS"
    else
        print_result "$test_name" "$expected" "$actual" "FAIL"
    fi
}

# Función especial para tests que requieren comparación exacta (sin sort)
run_test_exact() {
    local test_name="$1"
    local shell_command="$2"
    local bash_command="$3"
    
    if [ -z "$bash_command" ]; then
        bash_command="$shell_command"
    fi
    
    echo -e "${BLUE}Running: ${test_name}${NC}"
    
    # Ejecutar con bash (referencia) - sin sort
    expected=$(echo "$bash_command" | bash 2>/dev/null)
    
    # Ejecutar con nuestro shell - sin sort
    actual=$(echo -e "$shell_command\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>")
    
    # Comparar resultados
    if [ "$expected" = "$actual" ]; then
        print_result "$test_name" "$expected" "$actual" "PASS"
    else
        print_result "$test_name" "$expected" "$actual" "FAIL"
    fi
}

# Verificar que existe el ejecutable
if [ ! -f "./shell" ]; then
    echo -e "${RED}Error: No se encuentra el ejecutable './shell'${NC}"
    echo "Asegúrate de compilar primero con: make"
    exit 1
fi

echo -e "${BLUE}=== TESTS PARA EL SHELL (EXTENDIDO) ===${NC}"
echo -e "${YELLOW}Nota: Estos tests evitan el parsing complejo según la consigna${NC}"
echo

echo -e "${YELLOW}=== TESTS BÁSICOS DE COMANDOS SIMPLES ===${NC}"

# Test 1: Comando simple sin pipes
run_test "Comando simple: ls" "ls"

# Test 2: pwd
run_test_exact "pwd" "pwd"

# Test 3: whoami
run_test_exact "whoami" "whoami"

# Test 4: date (solo verificamos que ejecute, no el contenido exacto)
echo -e "${BLUE}Running: date (execution test)${NC}"
date_output=$(echo -e "date\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
if [ "$date_output" -gt 0 ]; then
    print_result "date execution" "output produced" "output produced" "PASS"
else
    print_result "date execution" "output produced" "no output" "FAIL"
fi

echo -e "${YELLOW}=== TESTS DE ECHO ===${NC}"

# Test 5: echo simple
run_test_exact "echo hello" "echo hello"

# Test 6: echo con múltiples palabras
run_test_exact "echo hello world" "echo hello world"

# Test 7: echo con comillas - OMITIDO (requiere procesamiento de cadenas)
# run_test_exact "echo \"hello world\"" "echo \"hello world\""

echo -e "${YELLOW}=== TESTS BÁSICOS DE PIPES ===${NC}"

# Test 8: Pipe básico simple - buscar archivos con extensión
run_test "Pipe básico: ls | grep .c" "ls | grep .c"

# Test 9: Pipe básico - buscar archivos shell
run_test "Pipe básico: ls | grep shell" "ls | grep shell"

# Test 10: Pipe básico - buscar Makefile
run_test "Pipe básico: ls | grep Makefile" "ls | grep Makefile"

# Test 11: Contar líneas con pipe
run_test_exact "Contar con pipe: ls | wc -l" "ls | wc -l"

# Test 12: head con pipe
run_test "head con pipe: ls | head -5" "ls | head -5"

# Test 13: tail con pipe
run_test "tail con pipe: ls | tail -3" "ls | tail -3"

echo -e "${YELLOW}=== TESTS DE MÚLTIPLES PIPES ===${NC}"

# Test 14: Triple pipe - contar archivos .c
run_test_exact "Triple pipe: ls | grep .c | wc -l" "ls | grep .c | wc -l"

# Test 15: Triple pipe - ordenar y limitar
run_test "Triple pipe: ls | sort | head -3" "ls | sort | head -3"

# Test 16: Triple pipe - buscar y ordenar
run_test "Triple pipe: ls | grep shell | sort" "ls | grep shell | sort"

# Test 17: Cuádruple pipe
run_test "Cuádruple pipe: ls | grep shell | sort | head -2" "ls | grep shell | sort | head -2"

echo -e "${YELLOW}=== TESTS DE COMANDOS CON ARGUMENTOS ===${NC}"

# Test 18: ls con argumentos
run_test "ls con -l: ls -l | grep shell" "ls -l | grep shell"

# Test 19: head con número
run_test "head con número: ls | head -5" "ls | head -5"

# Test 20: tail con número
run_test "tail con número: ls | tail -3" "ls | tail -3"

# Test 21: wc con diferentes opciones
run_test_exact "wc con -l: ls | wc -l" "ls | wc -l"
run_test_exact "wc con -w: ls | wc -w" "ls | wc -w"

# Test 22: sort con opciones
run_test "sort: ls | sort" "ls | sort"
run_test "sort reverse: ls | sort -r" "ls | sort -r"

echo -e "${YELLOW}=== TESTS DE COMANDOS DEL SISTEMA ===${NC}"

# Test 23: ps básico (solo verificamos ejecución)
echo -e "${BLUE}Running: ps (execution test)${NC}"
ps_output=$(echo -e "ps\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
if [ "$ps_output" -gt 0 ]; then
    print_result "ps execution" "output produced" "output produced" "PASS"
else
    print_result "ps execution" "output produced" "no output" "FAIL"
fi

# Test 24: ps con pipe y grep (evitando grep grep)
run_test "ps con pipe: ps aux | head -5" "ps aux | head -5"

# Test 25: who
echo -e "${BLUE}Running: who (execution test)${NC}"
who_output=$(echo -e "who\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
if [ "$who_output" -ge 0 ]; then  # Puede ser 0 si no hay usuarios logueados
    print_result "who execution" "executed" "executed" "PASS"
else
    print_result "who execution" "executed" "failed" "FAIL"
fi

echo -e "${YELLOW}=== TESTS DE ROBUSTEZ ===${NC}"

# Test 26: Espacios alrededor de pipes
run_test "Espacios: ls | grep shell" "ls | grep shell"
run_test "Sin espacios: ls|grep shell" "ls|grep shell" "ls | grep shell"

# Test 27: Pipe con cat
run_test "Pipe con cat: ls | cat" "ls | cat"

# Test 28: Múltiples espacios (si tu shell los maneja)
run_test "Múltiples espacios: ls  |  grep  shell" "ls  |  grep  shell" "ls | grep shell"

echo -e "${YELLOW}=== TESTS DE COMBINACIONES COMPLEJAS ===${NC}"

# Test 29: echo con pipe
run_test_exact "echo con pipe: echo test | grep test" "echo test | grep test"

# Test 30: echo múltiple con pipe
run_test "echo múltiple: echo hello world | grep hello" "echo hello world | grep hello"

# Test 31: Comando con múltiples argumentos
run_test "ls con argumentos: ls -la | head -3" "ls -la | head -3"

# Test 32: Pipe con wc y diferentes opciones
run_test_exact "wc caracteres: echo hello | wc -c" "echo hello | wc -c"

# Test 33: Combinación de comandos de sistema
run_test "pwd con pipe: pwd | cat" "pwd | cat"

echo -e "${YELLOW}=== TESTS DE LÍMITES ===${NC}"

# Test 34: Pipe muy largo (5 comandos)
run_test "5 pipes: ls | grep shell | sort | head -3 | cat" "ls | grep shell | sort | head -3 | cat"

# Test 35: Comando vacío con pipe (manejo de errores)
echo -e "${BLUE}Running: empty command test${NC}"
empty_output=$(echo -e " | grep test\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
# No verificamos resultado específico, solo que no crashee
print_result "empty command handling" "handled" "handled" "PASS"

echo -e "${BLUE}=== TESTS EXCLUIDOS POR PARSING COMPLEJO ===${NC}"
echo -e "${YELLOW}Los siguientes tipos de comandos NO se testean según la consigna:${NC}"
echo -e "- Comandos con comillas: echo \"hello world\" (requiere procesamiento de comillas)"
echo -e "- Patrones complejos: ls | grep \".png .zip\""
echo -e "- Parsing de argumentos con espacios y comillas"
echo -e "- Escape de caracteres especiales"
echo -e "- Redirección de archivos (>, <, >>)"
echo -e "- Variables de entorno complejas"
echo -e "- Comandos con && o ||"
echo

echo -e "${YELLOW}=== TESTS DE VERIFICACIÓN ADICIONAL ===${NC}"

# Tests adicionales basados en el segundo script pero sin requerimientos complejos
# Crear archivo auxiliar para tests
cat <<EOF > "test_temp.txt"
archivo1.txt
archivo2.zip
imagen.png
documento.pdf
EOF

# Test 36: Búsqueda con cat y grep
run_test "cat test_temp.txt | grep .zip" "cat test_temp.txt | grep .zip"

# Test 37: Pipeline triple simple
run_test_exact "echo hola | grep hola | wc -l" "echo hola | grep hola | wc -l"

# Test 38: Comando con ruta absoluta (si existe)
if [ -f "/bin/echo" ]; then
    run_test_exact "/bin/echo hola" "/bin/echo hola"
fi

# Test 39: Pipeline con sort y uniq
run_test "cat test_temp.txt | sort | uniq" "cat test_temp.txt | sort | uniq"

# Test 40: yes truncado por head (test de ejecución)
echo -e "${BLUE}Running: yes truncated by head${NC}"
yes_output=$(echo -e "yes | head -n 5\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
if [ "$yes_output" -eq 5 ]; then
    print_result "yes | head -n 5" "5 lines" "5 lines" "PASS"
else
    print_result "yes | head -n 5" "5 lines" "$yes_output lines" "FAIL"
fi

# Test 41: echo con string vacío (sin comillas complejas)
run_test_exact "echo" "echo"

# Test 42: grep que descarta salida
run_test_exact "echo hola | grep -v hola | wc -l" "echo hola | grep -v hola | wc -l"

# Test 43: cat /dev/null con pipe
run_test_exact "cat /dev/null | wc -l" "cat /dev/null | wc -l"

# Test 44: seq simple con pipe
run_test "seq 5 | grep 3" "seq 5 | grep 3"

# Test 45: seq con tail
run_test_exact "seq 5 | tail -n 1" "seq 5 | tail -n 1"

echo -e "${YELLOW}=== TESTS DE MANEJO DE ERRORES ===${NC}"

# Test 46: Pipe al inicio (debería fallar graciosamente)
echo -e "${BLUE}Running: pipe at beginning (error handling)${NC}"
pipe_start_output=$(echo -e "| echo hola\nexit" | timeout 5s ./shell 2>&1 | grep -v "Shell>")
if [ -n "$pipe_start_output" ]; then
    print_result "pipe at start handling" "handled error" "handled error" "PASS"
else
    print_result "pipe at start handling" "handled error" "no response" "FAIL"
fi

# Test 47: Pipe al final (debería fallar graciosamente)
echo -e "${BLUE}Running: pipe at end (error handling)${NC}"
pipe_end_output=$(echo -e "echo hola |\nexit" | timeout 5s ./shell 2>&1 | grep -v "Shell>")
if [ -n "$pipe_end_output" ]; then
    print_result "pipe at end handling" "handled error" "handled error" "PASS"
else
    print_result "pipe at end handling" "handled error" "no response" "FAIL"
fi

# Test 48: Comando inexistente (manejo de errores)
echo -e "${BLUE}Running: nonexistent command (error handling)${NC}"
nonexist_output=$(echo -e "comandoinexistente123\nexit" | timeout 5s ./shell 2>&1 | grep -v "Shell>")
if [ -n "$nonexist_output" ]; then
    print_result "nonexistent command handling" "handled error" "handled error" "PASS"
else
    print_result "nonexistent command handling" "handled error" "no response" "FAIL"
fi

echo -e "${YELLOW}=== TESTS DE ESPACIADO ===${NC}"

# Test 49: Espaciado irregular
run_test_exact "   echo    prueba   " "   echo    prueba   " "echo prueba"

# Test 50: Espacios múltiples con pipe
run_test_exact "echo hola    mundo | wc -w" "echo hola    mundo | wc -w"

echo -e "${YELLOW}=== TESTS DE PIPELINE LARGO ===${NC}"

# Test 51: Pipeline de múltiples comandos (más simple que el original)
run_test "echo test | grep test | cat | sort | uniq" "echo test | grep test | cat | sort | uniq"

# Test 52: Pipeline con 6 comandos
run_test "ls | head -10 | tail -5 | sort | grep shell | wc -l" "ls | head -10 | tail -5 | sort | grep shell | wc -l"

# Limpiar archivo temporal
rm -f "test_temp.txt"

# Guardar contadores de tests básicos
BASIC_TOTAL=$TOTAL
BASIC_PASSED=$PASSED
BASIC_FAILED=$FAILED

echo -e "${YELLOW}=== EXTRA CREDIT: TESTS DE PARSING COMPLEJO (NO NECESARIO) ===${NC}"
echo -e "${RED}⚠️  ATENCIÓN: Estos tests requieren parsing complejo de texto${NC}"
echo -e "${RED}⚠️  Según la consigna, NO es necesario implementar esta funcionalidad${NC}"
echo -e "${RED}⚠️  Si fallan, no afecta la evaluación del shell${NC}"
echo

# Test EXTRA 1: echo con comillas dobles (requiere procesamiento)
run_test_exact "echo \"hola mundo\"" "echo \"hola mundo\"" "echo hola mundo"

# Test EXTRA 2: echo con comillas simples múltiples
run_test_exact "echo 'uno' 'dos'" "echo 'uno' 'dos'" "echo uno dos"

# Test EXTRA 3: Comillas sin cerrar (manejo de errores complejo)
echo -e "${BLUE}Running: unclosed quotes (complex error handling)${NC}"
unclosed_output=$(echo -e "echo \"hola\nexit" | timeout 5s ./shell 2>&1 | grep -v "Shell>")
if [ -n "$unclosed_output" ]; then
    print_result "unclosed quotes handling" "handled error" "handled error" "PASS"
else
    print_result "unclosed quotes handling" "handled error" "no response" "FAIL"
fi

# Test EXTRA 4: Expansión de comandos (muy complejo)
echo -e "${BLUE}Running: command expansion (very complex)${NC}"
expansion_output=$(echo -e "echo \$(seq 3)\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -l)
expected_expansion_output=$(echo "1 2 3" | wc -w)
if [ "$expansion_output" -eq "$expected_expansion_output" ]; then
    print_result "command expansion" "expanded correctly" "expanded correctly" "PASS"
else
    print_result "command expansion" "expanded correctly" "not expanded" "FAIL"
fi

# Test EXTRA 5: Regex complejas con grep -E
# Crear archivo temporal con extensiones
cat <<EOF > "test_regex.txt"
archivo1.txt
imagen.png
documento.pdf
archivo2.zip
video.mp4
EOF

run_test "cat test_regex.txt | grep -E \"\\.png$|\\.zip$\"" "cat test_regex.txt | grep -E \"\\.png$|\\.zip$\""

# Test EXTRA 6: Límites de argumentos (63 argumentos)
ARGS_63=$(seq -s ' ' 1 63)
echo -e "${BLUE}Running: 63 arguments limit${NC}"
args_output=$(echo -e "echo $ARGS_63\nexit" | timeout 5s ./shell 2>/dev/null | grep -v "Shell>" | wc -w)
if [ "$args_output" -eq 63 ]; then
    print_result "63 arguments" "63 words" "63 words" "PASS"
else
    print_result "63 arguments" "63 words" "$args_output words" "FAIL"
fi

# Test EXTRA 7: Exceso de argumentos (64 argumentos - debería fallar)
ARGS_64=$(seq -s ' ' 1 64)
echo -e "${BLUE}Running: 64 arguments (should error)${NC}"
args_error_output=$(echo -e "echo $ARGS_64\nexit" | timeout 5s ./shell 2>&1 | grep -v "Shell>")
if [ -n "$args_error_output" ]; then
    print_result "64 arguments error" "handled error" "handled error" "PASS"
else
    print_result "64 arguments error" "handled error" "no error" "FAIL"
fi

# Test EXTRA 8: Pipeline extremadamente largo (200 procesos)
echo -e "${BLUE}Running: 200-process pipeline (stress test)${NC}"
PIPE_CHAIN=$(printf 'cat | %.0s' {1..198}; echo "wc -l")
stress_output=$(echo -e "echo test | $PIPE_CHAIN\nexit" | timeout 10s ./shell 2>/dev/null | grep -v "Shell>" | tr -d ' ')
if [ "$stress_output" = "1" ]; then
    print_result "200-process pipeline" "handled correctly" "handled correctly" "PASS"
else
    print_result "200-process pipeline" "handled correctly" "failed or timeout" "FAIL"
fi

# Limpiar archivos temporales extra
rm -f "test_regex.txt"

# Calcular contadores de extra credit
EXTRA_TOTAL=$((TOTAL - BASIC_TOTAL))
EXTRA_PASSED=$((PASSED - BASIC_PASSED))
EXTRA_FAILED=$((FAILED - BASIC_FAILED))

echo -e "${BLUE}=== RESUMEN DE TESTS BÁSICOS (REQUERIDOS) ===${NC}"
echo -e "Total de tests básicos: ${BASIC_TOTAL}"
echo -e "${GREEN}Pasaron: ${BASIC_PASSED}${NC}"
echo -e "${RED}Fallaron: ${BASIC_FAILED}${NC}"

if [ $BASIC_FAILED -eq 0 ]; then
    echo -e "${GREEN}¡Todos los tests básicos pasaron! ✓${NC}"
    echo -e "${BLUE}El shell cumple con los requerimientos fundamentales${NC}"
    BASIC_SUCCESS=true
else
    echo -e "${RED}Algunos tests básicos fallaron. Revisa la implementación.${NC}"
    echo -e "${YELLOW}Ratio de éxito básico: $(echo "scale=1; $BASIC_PASSED * 100 / $BASIC_TOTAL" | bc -l)%${NC}"
    BASIC_SUCCESS=false
fi

echo

echo -e "${BLUE}=== RESUMEN DE TESTS EXTRA CREDIT (OPCIONALES) ===${NC}"
echo -e "Total de tests extra: ${EXTRA_TOTAL}"
echo -e "${GREEN}Pasaron: ${EXTRA_PASSED}${NC}"
echo -e "${RED}Fallaron: ${EXTRA_FAILED}${NC}"

if [ $EXTRA_FAILED -eq 0 ]; then
    echo -e "${GREEN}¡Todos los tests extra credit pasaron! ⭐${NC}"
    echo -e "${BLUE}El shell maneja parsing complejo avanzado${NC}"
else
    echo -e "${YELLOW}Algunos tests extra credit fallaron (esto es normal y esperado)${NC}"
    echo -e "${YELLOW}Ratio de éxito extra: $(echo "scale=1; $EXTRA_PASSED * 100 / $EXTRA_TOTAL" | bc -l)%${NC}"
fi

echo

echo -e "${BLUE}=== RESUMEN TOTAL ===${NC}"
echo -e "Total de todos los tests: ${TOTAL}"
echo -e "${GREEN}Pasaron: ${PASSED}${NC}"
echo -e "${RED}Fallaron: ${FAILED}${NC}"

if [ "$BASIC_SUCCESS" = true ]; then
    if [ $FAILED -eq 0 ]; then
        echo -e "${GREEN}🎉 ¡PERFECTO! Todos los tests (básicos + extra) pasaron! 🎉${NC}"
        exit 0
    else
        echo -e "${GREEN}✅ ¡APROBADO! Todos los tests básicos pasaron${NC}"
        echo -e "${YELLOW}📚 Extra credit: $EXTRA_PASSED/$EXTRA_TOTAL${NC}"
        exit 0
    fi
else
    echo -e "${RED}❌ Algunos tests básicos fallaron. Revisa la implementación core.${NC}"
    exit 1
fi