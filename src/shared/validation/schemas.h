#pragma once

namespace rdws {
namespace validation {
namespace schemas {

// User schemas as constant strings
constexpr const char* USER_CREATE_SCHEMA = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "title": "Create User Schema",
    "description": "Schema for creating a new user",
    "properties": {
        "name": {
            "type": "string",
            "minLength": 2,
            "maxLength": 100,
            "description": "User's full name"
        },
        "email": {
            "type": "string",
            "format": "email",
            "maxLength": 255,
            "description": "User's email address"
        }
    },
    "required": ["name", "email"],
    "additionalProperties": false
})";

constexpr const char* USER_UPDATE_SCHEMA = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "title": "Update User Schema",
    "description": "Schema for updating an existing user",
    "properties": {
        "name": {
            "type": "string",
            "minLength": 2,
            "maxLength": 100,
            "description": "User's full name"
        },
        "email": {
            "type": "string",
            "format": "email",
            "maxLength": 255,
            "description": "User's email address"
        }
    },
    "additionalProperties": false
})";

constexpr const char* USER_QUERY_SCHEMA = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "title": "Query User Schema",
    "description": "Schema for user query parameters",
    "properties": {
        "page": {
            "type": "integer",
            "minimum": 1,
            "default": 1
        },
        "limit": {
            "type": "integer",
            "minimum": 1,
            "maximum": 100,
            "default": 10
        },
        "search": {
            "type": "string",
            "maxLength": 255
        },
        "sortBy": {
            "type": "string",
            "enum": ["id", "name", "email", "created_at"]
        },
        "sortOrder": {
            "type": "string",
            "enum": ["asc", "desc"],
            "default": "asc"
        }
    },
    "additionalProperties": false
})";

// Order schemas (for future use)
constexpr const char* ORDER_CREATE_SCHEMA = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "title": "Create Order Schema",
    "description": "Schema for creating a new order",
    "properties": {
        "user_id": {
            "type": "integer",
            "minimum": 1,
            "description": "ID of the user placing the order"
        },
        "product_name": {
            "type": "string",
            "minLength": 2,
            "maxLength": 200,
            "description": "Name of the product"
        },
        "quantity": {
            "type": "integer",
            "minimum": 1,
            "maximum": 1000,
            "description": "Quantity of the product"
        },
        "price": {
            "type": "number",
            "minimum": 0,
            "multipleOf": 0.01,
            "description": "Price per unit"
        }
    },
    "required": ["user_id", "product_name", "quantity", "price"],
    "additionalProperties": false
})";

constexpr const char* ORDER_UPDATE_SCHEMA = R"({
    "$schema": "http://json-schema.org/draft-07/schema#",
    "type": "object",
    "title": "Update Order Schema",
    "description": "Schema for updating an existing order",
    "properties": {
        "product_name": {
            "type": "string",
            "minLength": 2,
            "maxLength": 200,
            "description": "Name of the product"
        },
        "quantity": {
            "type": "integer",
            "minimum": 1,
            "maximum": 1000,
            "description": "Quantity of the product"
        },
        "price": {
            "type": "number",
            "minimum": 0,
            "multipleOf": 0.01,
            "description": "Price per unit"
        },
        "status": {
            "type": "string",
            "enum": ["pending", "processing", "shipped", "delivered", "cancelled"],
            "description": "Order status"
        }
    },
    "additionalProperties": false
})";

} // namespace schemas
} // namespace validation
} // namespace rdws
