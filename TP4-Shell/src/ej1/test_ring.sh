#!/bin/bash

# Script de pruebas para el programa anillo
# Este script ejecuta varios tests para verificar el comportamiento del programa

# Colores para mejor legibilidad
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Función para ejecutar tests
run_test() {
    local test_name=$1
    local n=$2
    local c=$3
    local s=$4
    local expected_result=$5
    
    echo -e "${YELLOW}Ejecutando test: $test_name${NC}"
    echo "Parámetros: n=$n, c=$c, s=$s"
    echo "Resultado esperado: $expected_result"
    
    # Ejecutar el programa y capturar la salida
    output=$(./anillo $n $c $s 2>&1)
    
    # Extraer el resultado final del output
    result=$(echo "$output" | grep "El resultado final es:" | awk '{print $5}')
    
    # Verificar el resultado
    if [ "$result" == "$expected_result" ]; then
        echo -e "${GREEN}✓ Test pasado correctamente${NC}"
    else
        echo -e "${RED}✗ Test fallido${NC}"
        echo "Resultado obtenido: $result"
        echo "Salida completa:"
        echo "$output"
    fi
    echo "----------------------------------------"
}

# Verificar si el programa existe
if [ ! -f "./anillo" ]; then
    echo -e "${RED}Error: El programa 'anillo' no existe en el directorio actual.${NC}"
    echo "Compilando el programa..."
    gcc -o anillo ring.c
    
    if [ ! -f "./anillo" ]; then
        echo -e "${RED}Error: No se pudo compilar el programa. Verifica que no haya errores en el código.${NC}"
        exit 1
    fi
fi

echo -e "${YELLOW}=== INICIANDO TESTS PARA EL PROGRAMA ANILLO ===${NC}"

# Test 1: Caso básico con 3 procesos, valor inicial 0, proceso inicial 1
# Resultado esperado: 3 (cada proceso incrementa una vez)
run_test "Caso básico - 3 procesos" 3 0 1 3

# Test 2: 5 procesos, valor inicial 10, proceso inicial 1
# Resultado esperado: 15 (10 + 5)
run_test "5 procesos - valor inicial 10" 5 10 1 15

# Test 3: 10 procesos, valor inicial 0, proceso inicial 5 (en medio)
# Resultado esperado: 10
run_test "10 procesos - inicio en medio" 10 0 5 10

# Test 4: Caso extremo con muchos procesos (20)
# Resultado esperado: 20 (si c=0)
run_test "Caso extremo - 20 procesos" 20 0 1 20

# Test 5: Inicio desde el último proceso del anillo
# Resultado esperado: 7 (si n=7, c=0, s=7)
run_test "Inicio desde último proceso" 7 0 7 7

# Test 6: Valor inicial negativo
# Resultado esperado: -7 + 10 = 3 (si n=10, c=-7, s=1)
run_test "Valor inicial negativo" 10 -7 1 3

# Test 7: Caso borde mínimo (n=3)
# Resultado esperado: 3 (si c=0)
run_test "Caso mínimo permitido (n=3)" 3 0 1 3

# Test 8: Prueba de parámetros incorrectos (n < 3)
echo -e "${YELLOW}Ejecutando test: Parámetros incorrectos (n < 3)${NC}"
echo "Parámetros: n=2, c=0, s=1"
echo "Resultado esperado: Error"
error_output=$(./anillo 2 0 1 2>&1)
if [[ $error_output == *"Error: n debe ser >= 3"* ]]; then
    echo -e "${GREEN}✓ Test pasado correctamente (error detectado)${NC}"
else
    echo -e "${RED}✗ Test fallido (no se detectó el error esperado)${NC}"
    echo "Salida:"
    echo "$error_output"
fi
echo "----------------------------------------"

# Test 9: Proceso de inicio fuera de rango (s > n)
echo -e "${YELLOW}Ejecutando test: Proceso de inicio fuera de rango (s > n)${NC}"
echo "Parámetros: n=5, c=0, s=6"
echo "Resultado esperado: Error"
error_output=$(./anillo 5 0 6 2>&1)
if [[ $error_output == *"Error: n debe ser >= 3 y el proceso de inicio debe estar entre 1 y n"* ]]; then
    echo -e "${GREEN}✓ Test pasado correctamente (error detectado)${NC}"
else
    echo -e "${RED}✗ Test fallido (no se detectó el error esperado)${NC}"
    echo "Salida:"
    echo "$error_output"
fi
echo "----------------------------------------"

echo -e "${YELLOW}=== TESTS COMPLETADOS ===${NC}"