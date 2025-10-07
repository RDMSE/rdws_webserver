/**
 * Database Configuration
 * Environment-specific database settings
 */

const config = {
    development: {
        host: process.env.DB_HOST || 'localhost',
        port: process.env.DB_PORT || 5432,
        database: process.env.DB_NAME || 'rdws_development',
        username: process.env.DB_USER || 'rdws_user',
        password: process.env.DB_PASS || 'rdws_pass123',
        dialect: 'postgresql',
        logging: console.log
    },

    production: {
        host: process.env.DB_HOST || 'localhost',
        port: process.env.DB_PORT || 5432,
        database: process.env.DB_NAME || 'rdws_production',
        username: process.env.DB_USER || 'rdws_user',
        password: process.env.DB_PASS || 'rdws_pass123',
        dialect: 'postgresql',
        logging: false
    }
};

module.exports = config;
