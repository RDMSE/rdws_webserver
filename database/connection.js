/**
 * Database Connection Helper
 * Simple PostgreSQL connection for microservices
 */

const { Pool } = require('pg');
const config = require('./config');

class DatabaseConnection {
    constructor(environment = 'development') {
        this.environment = environment;
        this.config = config[environment];
        this.pool = null;
    }

    async connect() {
        if (!this.pool) {
            this.pool = new Pool({
                host: this.config.host,
                port: this.config.port,
                database: this.config.database,
                user: this.config.username,
                password: this.config.password,
                max: 10,
                idleTimeoutMillis: 30000,
                connectionTimeoutMillis: 2000,
            });

            // Test connection
            try {
                await this.pool.query('SELECT NOW()');
                console.log(`Connected to database: ${this.config.database}`);
            } catch (error) {
                console.error('Database connection failed:', error.message);
                throw error;
            }
        }
        return this.pool;
    }

    async query(text, params) {
        const pool = await this.connect();
        return pool.query(text, params);
    }

    async close() {
        if (this.pool) {
            await this.pool.end();
            this.pool = null;
            console.log('🔌 Database connection closed');
        }
    }
}

module.exports = DatabaseConnection;
