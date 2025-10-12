#!/bin/bash

# Post-Deploy Verification Script
# Verifies that the API Gateway is working correctly

set -e

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

print_status() { echo -e "${BLUE}[INFO]${NC} $1"; }
print_success() { echo -e "${GREEN}${NC} $1"; }
print_error() { echo -e "${RED}${NC} $1"; }
print_warning() { echo -e "${YELLOW}${NC} $1"; }

# Configuration
PORT=${1:-8080}
HOST=${2:-localhost}
BASE_URL="http://${HOST}:${PORT}"

print_status "Starting API Gateway verification..."
print_status "Testing: $BASE_URL"
echo ""

# Function to test endpoint
test_endpoint() {
    local endpoint=$1
    local description=$2
    local expected_status=${3:-200}

    print_status "Testing: $endpoint - $description"

    response=$(curl -s -w "HTTP_STATUS:%{http_code}" "$BASE_URL$endpoint" 2>/dev/null)

    if [ $? -eq 0 ]; then
        status_code=$(echo "$response" | grep -o "HTTP_STATUS:[0-9]*" | cut -d: -f2)
        body=$(echo "$response" | sed 's/HTTP_STATUS:[0-9]*$//')

        if [ "$status_code" = "$expected_status" ]; then
            print_success "$endpoint - Status: $status_code"

            # Specific checks per endpoint
            case "$endpoint" in
                "/health")
                    if echo "$body" | grep -q "\"status\""; then
                        print_success "Health endpoint returns valid JSON"
                    else
                        print_warning "Health endpoint JSON might be invalid"
                    fi
                    ;;
                "/users")
                    if echo "$body" | grep -q "\"users\""; then
                        user_count=$(echo "$body" | grep -o "\"total\":[0-9]*" | cut -d: -f2)
                        print_success "Users endpoint returns $user_count users"
                    else
                        print_warning "Users endpoint might not return expected format"
                    fi
                    ;;
                "/orders")
                    if echo "$body" | grep -q "\"orders\""; then
                        order_count=$(echo "$body" | grep -o "\"total\":[0-9]*" | cut -d: -f2)
                        print_success "Orders endpoint returns $order_count orders"
                    else
                        print_warning "Orders endpoint might not return expected format"
                    fi
                    ;;
            esac
        else
            print_error "$endpoint - Expected: $expected_status, Got: $status_code"
            return 1
        fi
    else
        print_error "$endpoint - Connection failed"
        return 1
    fi
}

# Function to check response time
check_performance() {
    local endpoint=$1
    local description=$2

    print_status "Performance test: $endpoint"

    time_total=$(curl -s -w "%{time_total}" -o /dev/null "$BASE_URL$endpoint" 2>/dev/null)

    if [ $? -eq 0 ]; then
        # Convert to milliseconds and round
        time_ms_int=$(echo "$time_total * 1000 / 1" | bc 2>/dev/null || echo "0")

        if [ "$time_ms_int" -lt 1000 ]; then
            print_success "$description - Response time: ${time_ms_int}ms"
        elif [ "$time_ms_int" -lt 3000 ]; then
            print_warning "$description - Response time: ${time_ms_int}ms (could be better)"
        else
            print_error "$description - Response time: ${time_ms_int}ms (too slow)"
        fi
    else
        print_error "$description - Performance test failed"
    fi
}

# Check if service is running
print_status "Basic connectivity test..."
if curl -s --connect-timeout 5 "$BASE_URL/health" > /dev/null; then
    print_success "Service is reachable"
else
    print_error "Service is not reachable at $BASE_URL"
    print_error "Please check if:"
    echo "  • Service is running"
    echo "  • Port $PORT is open"
    echo "  • Firewall allows connections"
    exit 1
fi

echo ""
print_status "Testing API endpoints..."

# Main endpoint tests
TESTS_PASSED=0
TESTS_TOTAL=0

# Health check
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/health" "Health check"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# API docs
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/api-docs" "API documentation"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Users endpoints
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/users" "List users"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/users/1" "Get user by ID"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Orders endpoints
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/orders" "List orders"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/orders/1" "Get order by ID"; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

# Error handling tests
TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/users/invalid" "Invalid user ID" 400; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

TESTS_TOTAL=$((TESTS_TOTAL + 1))
if test_endpoint "/nonexistent" "404 handling" 404; then
    TESTS_PASSED=$((TESTS_PASSED + 1))
fi

echo ""
print_status "Performance tests..."

# Performance tests
if command -v bc > /dev/null; then
    check_performance "/health" "Health check"
    check_performance "/users" "Users list"
    check_performance "/orders" "Orders list"
else
    print_warning "bc not available, skipping performance tests"
fi

echo ""
print_status "Security tests..."

# Check security headers
SECURITY_RESPONSE=$(curl -s -I "$BASE_URL/health" 2>/dev/null)

if echo "$SECURITY_RESPONSE" | grep -qi "x-content-type-options"; then
    print_success "Security headers present (X-Content-Type-Options)"
else
    print_warning "Missing security header: X-Content-Type-Options"
fi

if echo "$SECURITY_RESPONSE" | grep -qi "access-control-allow-origin"; then
    print_success "CORS headers present"
else
    print_warning "CORS headers not found"
fi

echo ""
print_status "Testing concurrent requests..."

# Basic concurrency test
if command -v xargs > /dev/null; then
    print_status "Running 5 concurrent requests..."
    echo -e "/health\n/users\n/orders\n/health\n/api-docs" | xargs -I {} -P 5 curl -s "$BASE_URL{}" > /dev/null
    if [ $? -eq 0 ]; then
        print_success "Concurrent requests handled successfully"
    else
        print_warning "Some concurrent requests failed"
    fi
else
    print_warning "xargs not available, skipping concurrency test"
fi

echo ""
print_status "Summary:"
echo "  Tests passed: $TESTS_PASSED/$TESTS_TOTAL"

if [ $TESTS_PASSED -eq $TESTS_TOTAL ]; then
    print_success "All tests passed! API Gateway is working correctly."

    echo ""
    print_status "Service endpoints:"
    echo "  Health:     $BASE_URL/health"
    echo "  Users:      $BASE_URL/users"
    echo "  Orders:     $BASE_URL/orders"
    echo "  API Docs:   $BASE_URL/api-docs"

    echo ""
    print_status "Quick test commands:"
    echo "  curl $BASE_URL/health"
    echo "  curl $BASE_URL/users | jq ."
    echo "  curl $BASE_URL/orders | jq ."

    exit 0
else
    print_error "Some tests failed. Please check the logs and configuration."

    echo ""
    print_status "Troubleshooting tips:"
    echo "  • Check service logs"
    echo "  • Verify microservices are compiled"
    echo "  • Ensure all dependencies are installed"
    echo "  • Check firewall settings"

    exit 1
fi
