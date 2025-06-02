#!/bin/bash
# filepath: test_ring.sh

# Script de tests para el programa ring

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' 

# Contadores
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Función para imprimir resultados
print_test_result() {
    local test_name="$1"
    local expected="$2"
    local actual="$3"
    local status="$4"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN} PASS${NC}: $test_name"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED} FAIL${NC}: $test_name"
        echo -e "  Expected: $expected"
        echo -e "  Actual: $actual"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Función para ejecutar test 
run_test() {
    local test_name="$1"
    local args="$2"
    local expected_result="$3"
    
    echo -e "${YELLOW}Running:${NC} $test_name"
    
    # Ejecutar el programa y capturar output
    local output=$(./ring $args 2>&1)
    local exit_code=$?
    
   
    local actual_result=$(echo "$output" | grep "Resultado final:" | grep -o '\-\?[0-9]\+$')                                                       
    
    # Verificar si el test pasó
    if [ "$actual_result" = "$expected_result" ] && [ $exit_code -eq 0 ]; then
        print_test_result "$test_name" "$expected_result" "$actual_result" "PASS"
    else
        print_test_result "$test_name" "$expected_result" "$actual_result (exit: $exit_code)" "FAIL"
    fi
}

# Función para test de error

run_error_test() {
    local test_name="$1"
    local args="$2"
    local should_fail="$3"
    
    echo -e "${YELLOW}Running:${NC} $test_name"
    
    # Ejecutar el programa y capturar output y exit code por separado
    local temp_file="/tmp/ring_error_test_$$"
    
    # Ejecutar y capturar exit code correctamente
    if [ -z "$args" ]; then
        # Sin argumentos
        ./ring > "$temp_file" 2>&1
        local exit_code=$?
    else
        # Con argumentos
        ./ring $args > "$temp_file" 2>&1
        local exit_code=$?
    fi
    
    local output=$(cat "$temp_file")
    rm -f "$temp_file"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    if [ "$should_fail" = "true" ] && [ $exit_code -ne 0 ]; then
        echo -e "${GREEN} PASS${NC}: $test_name (correctly failed with exit code $exit_code)"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    elif [ "$should_fail" = "false" ] && [ $exit_code -eq 0 ]; then
        echo -e "${GREEN} PASS${NC}: $test_name (correctly succeeded)"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        echo -e "${RED} FAIL${NC}: $test_name"
        echo -e "  Expected failure: $should_fail, got exit code: $exit_code"
        echo -e "  Output: $output"
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
}

# Compilar el programa
echo -e "${YELLOW}Compilando ring.c...${NC}"
gcc ring.c -o ring
if [ $? -ne 0 ]; then
    echo -e "${RED}Error al compilar ring.c${NC}"
    exit 1
fi

echo -e "${GREEN}Compilación exitosa${NC}\n"

echo "=============================================="
echo "           INICIANDO TESTS DEL RING"
echo "=============================================="

# ==========================================
# TESTS BÁSICOS DE FUNCIONALIDAD
# ==========================================
echo -e "\n${YELLOW}=== TESTS BÁSICOS DE FUNCIONALIDAD ===${NC}"

# Test 1: Caso básico - 3 procesos, valor inicial 5, empieza en proceso 1
# Esperado: 5 + 3 = 8
run_test "Caso básico (3 procesos, valor 5)" "3 5 1" "8"

# Test 2: Tres procesos mínimo (tu programa requiere n >= 3)
# Esperado: 0 + 3 = 3
run_test "Tres procesos mínimo" "3 0 1" "3"

# Test 3: Cuatro procesos
# Esperado: 10 + 4 = 14
run_test "Cuatro procesos" "4 10 1" "14"

# Test 4: Muchos procesos
# Esperado: 1 + 10 = 11
run_test "Muchos procesos (10)" "10 1 1" "11"

# ==========================================
# TESTS DE VALORES LÍMITE
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE VALORES LÍMITE ===${NC}"

# Test 5: Valor inicial 0 
run_test "Valor inicial 0" "3 0 1" "3"

# Test 6: Valor inicial negativo 
run_test "Valor inicial negativo" "3 -5 1" "-2"

# Test 7: Valor inicial grande 
run_test "Valor inicial grande" "5 1000 1" "1005"

# Test 8: Muchos procesos con valor 0 
run_test "Muchos procesos valor 0" "20 0 1" "20"

# ==========================================
# TESTS DE CASOS EXTREMOS
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE CASOS EXTREMOS ===${NC}"

# Test 9: Máximo número razonable de procesos
run_test "Muchos procesos (50)" "50 0 1" "50"

# Test 10: Valor muy negativo
run_test "Valor muy negativo" "3 -100 1" "-97"

# Test 11: Combinación extrema
run_test "Caso extremo combinado" "15 -10 1" "5"

# ==========================================
# TESTS DE VALIDACIÓN DE ARGUMENTOS
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE VALIDACIÓN DE ARGUMENTOS ===${NC}"

# Test 12: Sin argumentos 
run_error_test "Sin argumentos" "" "true"

# Test 13: Pocos argumentos 
run_error_test "Pocos argumentos (1)" "3" "true"

# Test 14: Pocos argumentos (2) 
run_error_test "Pocos argumentos (2)" "3 5" "true"

# Test 15: Muchos argumentos 
run_error_test "Muchos argumentos" "3 5 1 extra" "true"

# Test 16: Argumentos no numéricos 
run_error_test "Argumentos no numéricos (n)" "abc 5 1" "true"
run_error_test "Argumentos no numéricos (c)" "3 abc 1" "true"  
run_error_test "Argumentos no numéricos (s)" "3 5 abc" "true"

# Test 17-18: n inválido (< 3) 
run_error_test "Número de procesos inválido (1)" "1 5 1" "true"
run_error_test "Número de procesos inválido (2)" "2 5 1" "true"

# Test 19-20: start fuera de rango 
run_error_test "Proceso inicial 0 (inválido)" "5 10 0" "true"
run_error_test "Proceso inicial fuera de rango" "5 10 6" "true"

# ==========================================
# TESTS DE CASOS BORDE MATEMÁTICOS
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE CASOS BORDE MATEMÁTICOS ===${NC}"

# Test 19: Overflow potencial (valores grandes)
run_test "Valores grandes" "5 2147483640 1" "2147483645"

# Test 20: Underflow potencial
run_test "Valores muy negativos" "3 -2147483645 1" "-2147483642"

# ==========================================
# TESTS DE ROBUSTEZ
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE ROBUSTEZ ===${NC}"

# Test 21: Cero procesos 
run_error_test "Cero procesos" "0 5 0" "true"

# Test 22: Número negativo de procesos
run_error_test "Número negativo de procesos" "-1 5 0" "true"

# ==========================================
# TESTS DE CONSISTENCIA
# ==========================================
echo -e "\n${YELLOW}=== TESTS DE CONSISTENCIA ===${NC}"

# Test 23: Mismos parámetros, múltiples ejecuciones
echo -e "${YELLOW}Testing consistency...${NC}"
result1=$(./ring 4 7 1 2>&1 | grep "Resultado final:" | grep -o '\-\?[0-9]\+$')
result2=$(./ring 4 7 1 2>&1 | grep "Resultado final:" | grep -o '\-\?[0-9]\+$')
result3=$(./ring 4 7 1 2>&1 | grep "Resultado final:" | grep -o '\-\?[0-9]\+$')                                                              

TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ "$result1" = "$result2" ] && [ "$result2" = "$result3" ] && [ "$result1" = "11" ]; then
    echo -e "${GREEN} PASS${NC}: Consistencia en múltiples ejecuciones"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    echo -e "${RED} FAIL${NC}: Inconsistencia detectada"
    echo -e "  Resultado 1: $result1"
    echo -e "  Resultado 2: $result2"
    echo -e "  Resultado 3: $result3"
    FAILED_TESTS=$((FAILED_TESTS + 1))
fi

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

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "\n${GREEN}¡TODOS LOS TESTS PASARON!${NC}"
    exit 0
else
    echo -e "\n${RED} Algunos tests fallaron. Revisa la implementación.${NC}"
    exit 1
fi