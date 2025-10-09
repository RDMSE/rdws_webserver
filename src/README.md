# 🚀 Adding New Microservices to API Gateway

This guide explains how to add new microservices to the modular API Gateway architecture.

## 📁 Project Structure

```
src/
├── types/
│   └── index.ts              # Shared types and interfaces
├── routes/
│   ├── BaseRouter.ts         # Abstract base class for all routers
│   ├── users.routes.ts       # Users microservice routes
│   ├── orders.routes.ts      # Orders microservice routes
│   ├── products.routes.ts    # Example: Products microservice routes
│   └── index.ts              # Exports all routers
├── services/                 # C++ Microservices (reorganized location)
│   ├── users/
│   │   ├── main.cpp         # Users C++ implementation
│   │   └── CMakeLists.txt
│   ├── orders/
│   │   ├── main.cpp         # Orders C++ implementation
│   │   └── CMakeLists.txt
│   └── CMakeLists.txt       # Services build configuration
└── shared/                   # Shared utilities and common code
```

## ✨ How to Add a New Microservice

### 1. Create the Router File

Create a new file in `src/routes/` following the naming pattern: `{service-name}.routes.ts`

```typescript
// src/routes/inventory.routes.ts
import { Router, Request, Response } from 'express';
import { BaseRouter } from './BaseRouter';
import { RouteConfig, CallMicroserviceFunction, HandleServiceErrorFunction } from '../types';

export class InventoryRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'inventory',           // Name of your C++ executable
        basePath: '/inventory',             // Base URL path
        endpoints: [
            {
                path: '',                   // GET /inventory
                method: 'GET',
                handler: 'getAllItems'
            },
            {
                path: '/:id',              // GET /inventory/:id
                method: 'GET',
                handler: 'getItemById',
                validation: (req) => this.validateNumericId(req.params.id, 'Item ID')
            }
        ]
    };

    public setupRoutes(
        callMicroservice: CallMicroserviceFunction,
        handleServiceError: HandleServiceErrorFunction
    ): Router {
        // GET /inventory
        this.router.get('/', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler('GET', '/inventory');
            await handler(req, res, callMicroservice, handleServiceError);
        });

        // GET /inventory/:id
        this.router.get('/:id', async (req: Request, res: Response): Promise<void> => {
            const handler = this.createRouteHandler(
                'GET',
                '/inventory/:id',
                (req) => this.validateNumericId(req.params.id, 'Item ID')
            );
            await handler(req, res, callMicroservice, handleServiceError);
        });

        return this.router;
    }
}
```

### 2. Export the Router

Add your router to `src/routes/index.ts`:

```typescript
export { InventoryRouter } from './inventory.routes';
```

### 3. Register in API Gateway

Add your router to the main `src/api-gateway/api-gateway.ts`:

```typescript
import {
    UsersRouter,
    OrdersRouter,
    UserOrdersRouter,
    InventoryRouter,  // Add this import
    // ... other imports
} from './src/routes';

// Add to the microserviceRouters array
const microserviceRouters: MicroserviceRouter[] = [
    new UsersRouter(),
    new OrdersRouter(),
    new UserOrdersRouter(),
    new InventoryRouter()  // Add this line
];
```

### 4. Create the C++ Microservice

Ensure you have a corresponding C++ executable at:
```
build/src/services/inventory/inventory_service
```

That accepts command line arguments: `method` and `path`

**That's it!** 🎉 Your new microservice will be automatically:
- ✅ Registered with routes
- ✅ Listed in `/health` endpoint
- ✅ Documented in `/api-docs`
- ✅ Available at startup logs

> **Note**: With the reorganized structure, C++ microservices are now located in `src/services/` for better project organization.

## 🛠️ Advanced Features

### Custom Validation

Add custom validation logic:

```typescript
validation: (req: Request) => {
    if (!req.params.id.match(/^[A-Z]{3}\\d{3}$/)) {
        return 'ID must be format ABC123';
    }
    return null; // null = valid
}
```

### POST/PUT/DELETE Routes

```typescript
// POST /inventory
this.router.post('/', async (req: Request, res: Response): Promise<void> => {
    const handler = this.createRouteHandler('POST', '/inventory');
    await handler(req, res, callMicroservice, handleServiceError);
});
```

### Cross-Service Routes

Routes that use one service but appear under another path:

```typescript
export class UserInventoryRouter extends BaseRouter {
    public routeConfig: RouteConfig = {
        serviceName: 'inventory',  // Uses inventory service
        basePath: '/users',        // But appears under /users
        endpoints: [
            {
                path: '/:userId/inventory',
                method: 'GET',
                handler: 'getUserInventory'
            }
        ]
    };
    // ... implementation
}
```

## 🧪 Testing

After adding a new microservice:

1. **Type Check**: `npm run type-check`
2. **Build**: `npm run build:ts`
3. **Test**: `npm test`
4. **Run**: `npm run dev`

## 📋 Benefits of This Architecture

- ✅ **Scalable**: Easy to add new microservices
- ✅ **Type Safe**: Full TypeScript support
- ✅ **Consistent**: Standardized patterns via BaseRouter
- ✅ **Automatic**: Documentation and health checks
- ✅ **Modular**: Each service in its own file
- ✅ **Testable**: Isolated and mockable components

## 🎯 Example Use Cases

- **E-commerce**: products, cart, payments, shipping
- **Social**: users, posts, comments, notifications
- **Business**: employees, departments, projects, reports
- **IoT**: devices, sensors, data, analytics

Each microservice becomes a simple addition following the same pattern! 🚀
