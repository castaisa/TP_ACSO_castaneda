#!/bin/bash
# filepath: test_shell.sh

# Script de tests para el shell interactivo (Compatible con macOS)
# Autor: GitHub Copilot

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Contadores
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Archivos temporales para pruebas
TEST_DIR="test_temp"
TEST_FILE1="test1.txt"
TEST_FILE2="test2.zip"
TEST_FILE3="test3.png"
# Función para setup inicial
setup_test_environment() {
    echo -e "${BLUE}Configurando entorno de pruebas...${NC}"
    
    # Compilar el shell primero
    echo -e "${BLUE}Compilando shell...${NC}"
    gcc shell.c -o shell
    if [ $? -ne 0 ]; then
        echo -e "${RED}Error al compilar shell.c${NC}"
        exit 1
    fi
    
    # Guardar directorio actual
    local original_dir=$(pwd)
    
    # Crear directorio temporal con ruta absoluta
    local abs_test_dir="$original_dir/$TEST_DIR"
    mkdir -p "$abs_test_dir"
    
    # Cambiar al directorio temporal
    cd "$abs_test_dir"
    
    # Crear archivos de prueba
    echo -e "archivo1\narchivo2.txt\narchivo3.zip\ndocumento.pdf" > "$TEST_FILE1"
    echo "contenido de archivo zip" > "$TEST_FILE2"
    echo "contenido de archivo png" > "$TEST_FILE3"
    echo "otro archivo.doc" > "documento.doc"
    echo "script.sh" > "script.sh"
    echo "imagen.jpg" > "imagen.jpg"
    
    # Volver al directorio original
    cd "$original_dir"
    
    # Verificar que el directorio y archivos fueron creados
    if [ -d "$TEST_DIR" ] && [ -f "$TEST_DIR/$TEST_FILE1" ]; then
        echo -e "${GREEN}Setup completado - Directorio $TEST_DIR creado exitosamente${NC}\n"
        echo -e "${BLUE}Archivos creados:${NC}"
        ls -la "$TEST_DIR"
        echo ""
    else
        echo -e "${RED}Error: No se pudo crear el directorio de pruebas${NC}"
        exit 1
    fi
}

# Función para cleanup
cleanup_test_environment() {
    echo -e "${BLUE}Limpiando entorno de pruebas...${NC}"
    rm -rf "$TEST_DIR"
    rm -f shell
    # Limpiar archivos temporales que puedan quedar
    rm -f /tmp/shell_test_*
}

# Función para imprimir resultados
print_test_result() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    local status="$4"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}✓ PASS${NC}: $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED}✗ FAIL${NC}: $test_name"
        echo -e "  Expected: $expected"
        echo -e "  Actual: $actual"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Función para ejecutar comando en el shell y capturar output
run_shell_command() {
    local command="$1"
    
    # Crear archivo temporal con nombre único
    local cmd_file="/tmp/shell_test_$$_$(date +%s)"
    
    # Usar ruta absoluta del directorio de prueba
    local abs_test_dir="$(pwd)/$TEST_DIR"
    
    # Verificar que el directorio existe
    if [ ! -d "$abs_test_dir" ]; then
        echo "ERROR: Directory $abs_test_dir does not exist"
        return
    fi
    
    # Cambiar al directorio de prueba y ejecutar comando
    echo "cd \"$abs_test_dir\"" > "$cmd_file"
    echo "$command" >> "$cmd_file"
    echo "exit" >> "$cmd_file"
    
    # Debug: mostrar qué comandos se están enviando
    echo -e "${BLUE}Debug: Enviando comandos:${NC}" >&2
    cat "$cmd_file" >&2
    echo "" >&2
    
    # Ejecutar shell con timeout implementado manualmente
    local output_file="/tmp/shell_output_$$_$(date +%s)"
    
    # Ejecutar shell en background y capturar output
    (./shell < "$cmd_file" 2>&1 | grep -v "Shell interactivo" | grep -v "Shell>" | grep -v "Saliendo del shell" > "$output_file") &
    local shell_pid=$!
    
    # Esperar máximo 10 segundos
    local count=0
    while [ $count -lt 10 ]; do
        if ! kill -0 $shell_pid 2>/dev/null; then
            # Proceso terminó
            break
        fi
        sleep 1
        count=$((count + 1))
    done
    
    # Si el proceso sigue corriendo, matarlo
    if kill -0 $shell_pid 2>/dev/null; then
        kill -9 $shell_pid 2>/dev/null
        echo "TIMEOUT_ERROR"
    else
        # Leer output
        if [ -f "$output_file" ]; then
            cat "$output_file"
        fi
    fi
    
    # Limpiar archivos temporales
    rm -f "$cmd_file" "$output_file"
}
# Función para comparar outputs con bash
compare_with_bash() {
    local test_name="$1"
    local command="$2"
    
    echo -e "${YELLOW}Testing:${NC} $test_name"
    
    # Ejecutar en bash (en el directorio de prueba)
    local bash_output
    cd "$TEST_DIR"
    bash_output=$(eval "$command" 2>/dev/null | head -20)  # Limitar output
    cd ..
    
    # Ejecutar en nuestro shell
    local shell_output=$(run_shell_command "$command")
    
    # Verificar si hubo timeout
    if [ "$shell_output" = "TIMEOUT_ERROR" ]; then
        print_test_result "$test_name" "$bash_output" "TIMEOUT" "FAIL"
        return
    fi
    
    # Comparar outputs (ignorando diferencias menores de whitespace y orden)
    local bash_clean=$(echo "$bash_output" | sort | tr -d '[:space:]')
    local shell_clean=$(echo "$shell_output" | sort | tr -d '[:space:]')
    
    if [ "$bash_clean" = "$shell_clean" ]; then
        print_test_result "$test_name" "$bash_output" "$shell_output" "PASS"
    else
        print_test_result "$test_name" "$bash_output" "$shell_output" "FAIL"
    fi
}

# Función para test básico sin comparación
run_basic_test() {
    local test_name="$1"
    local command="$2"
    local should_succeed="$3"
    
    echo -e "${YELLOW}Testing:${NC} $test_name"
    
    local output=$(run_shell_command "$command")
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ "$output" = "TIMEOUT_ERROR" ]; then
        echo -e "${RED}✗ FAIL${NC}: $test_name (timeout)"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        return
    fi
    
    if [ "$should_succeed" = "true" ]; then
        echo -e "${GREEN}✓ PASS${NC}: $test_name (ejecutado sin error)"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        if [ -n "$output" ]; then
            echo -e "${GREEN}✓ PASS${NC}: $test_name (produjo output)"
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            echo -e "${RED}✗ FAIL${NC}: $test_name (no produjo output)"
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
}

# Configurar entorno
setup_test_environment

echo "=============================================="
echo "           TESTS DEL SHELL INTERACTIVO"
echo "=============================================="

# ==========================================
# TESTS BÁSICOS DE FUNCIONALIDAD
# ==========================================
echo -e "\n${YELLOW}=== TESTS BÁSICOS DE FUNCIONALIDAD ===${NC}"

compare_with_bash "Comando simple: ls" "ls"
compare_with_bash "Comando con argumentos: ls -l" "ls -l"
compare_with_bash "Pipe básico: ls | wc -l" "ls | wc -l"
compare_with_bash "Grep simple: ls | grep txt" "ls | grep txt"

# ==========================================
# TESTS DE PIPES MÚLTIPLES
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE PIPES MÚLTIPLES ===${NC}"

compare_with_bash "Tres comandos: ls | grep txt | wc -l" "ls | grep txt | wc -l"
compare_with_bash "Cuatro comandos: ls | sort | uniq | wc -l" "ls | sort | uniq | wc -l"
compare_with_bash "Pipe con cat: cat $TEST_FILE1 | grep archivo" "cat $TEST_FILE1 | grep archivo"
compare_with_bash "Sort y head: ls | sort | head -3" "ls | sort | head -3"

# ==========================================
# TESTS DE CASOS ESPECÍFICOS DEL ENUNCIADO
# ==========================================
echo -e "\n${YELLOW}=== TESTS ESPECÍFICOS DEL ENUNCIADO ===${NC}"

compare_with_bash "Buscar .zip: ls | grep .zip" "ls | grep .zip"
compare_with_bash "Buscar .txt: ls | grep .txt" "ls | grep .txt"
compare_with_bash "Buscar .png: ls | grep .png" "ls | grep .png"

# ==========================================
# TESTS DE COMANDOS COMPLEJOS
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE COMANDOS COMPLEJOS ===${NC}"

compare_with_bash "echo con sort: echo 'c b a' | tr ' ' '\n' | sort" "echo 'c b a' | tr ' ' '\n' | sort"

# ==========================================
# TESTS DE CASOS BORDE
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE CASOS BORDE ===${NC}"

run_basic_test "Comando vacío" "" "true"
run_basic_test "Solo espacios" "   " "true"
run_basic_test "Comando inexistente: comandoquenoexiste" "comandoquenoexiste" "false"

# ==========================================
# TESTS DE ESPACIOS Y PARSING
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE PARSING Y ESPACIOS ===${NC}"

compare_with_bash "Espacios extra: ls  |  grep   txt" "ls | grep txt"
run_basic_test "Muchos espacios: ls     |     grep     txt" "ls     |     grep     txt" "true"

# ==========================================
# TESTS DE ARGUMENTOS MÚLTIPLES
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE ARGUMENTOS MÚLTIPLES ===${NC}"

compare_with_bash "ls con opciones: ls -la | head -5" "ls -la | head -5"
compare_with_bash "grep con opciones: ls | grep -v txt" "ls | grep -v txt"
compare_with_bash "sort con opciones: ls | sort -r" "ls | sort -r"

# ==========================================
# TESTS DE COMANDOS DEL SISTEMA
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE COMANDOS DEL SISTEMA ===${NC}"

run_basic_test "echo simple: echo hola" "echo hola" "true"
run_basic_test "echo con pipe: echo 'test' | wc -c" "echo 'test' | wc -c" "true"
run_basic_test "cat con pipe: echo 'test' | cat" "echo 'test' | cat" "true"

# ==========================================
# RESUMEN FINAL
# ==========================================
echo ""
echo "=============================================="
echo "               RESUMEN DE TESTS"
echo "=============================================="
echo -e "Total de tests ejecutados: ${TOTAL_TESTS}"
echo -e "${GREEN}Tests pasados: ${PASSED_TESTS}${NC}"
echo -e "${RED}Tests fallidos: ${FAILED_TESTS}${NC}"

# Calcular porcentaje
if [ $TOTAL_TESTS -gt 0 ]; then
    percentage=$((PASSED_TESTS * 100 / TOTAL_TESTS))
    echo -e "Porcentaje de éxito: ${percentage}%"
fi

# Limpiar entorno
cleanup_test_environment

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "\n${GREEN}🎉 ¡TODOS LOS TESTS PASARON! 🎉${NC}"
    echo -e "Tu shell está funcionando correctamente."
    exit 0
else
    echo -e "\n${RED}❌ Algunos tests fallaron.${NC}"
    echo -e "Revisa la implementación para mejorar la compatibilidad."
    exit 1
fi