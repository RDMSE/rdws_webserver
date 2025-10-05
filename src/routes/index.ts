/**
 * Routes Index
 * 
 * Exports all microservice routers for easy importing
 */

export { BaseRouter } from './BaseRouter';
export { UsersRouter } from './users.routes';
export { OrdersRouter, UserOrdersRouter } from './orders.routes';
export { ProductsRouter } from './products.routes';

// Re-export types for convenience
export * from '../types';