-- Development Seed Data
-- Sample data for development environment

-- Insert categories
INSERT INTO categories (name, description) VALUES
('Electronics', 'Electronic devices and gadgets'),
('Books', 'Books and educational materials'),
('Clothing', 'Apparel and accessories')
ON CONFLICT (name) DO NOTHING;

-- Insert sample users
INSERT INTO users (name, email) VALUES
('John Doe', 'john@example.com'),
('Jane Smith', 'jane@example.com'),
('Bob Johnson', 'bob@example.com'),
('Alice Brown', 'alice@example.com')
ON CONFLICT (email) DO NOTHING;

-- Insert sample products
INSERT INTO products (name, description, price, category_id, stock_quantity) VALUES
('Laptop Pro', 'High-performance laptop for professionals', 999.99, 1, 10),
('Wireless Mouse', 'Ergonomic wireless mouse', 29.99, 1, 50),
('Programming Book', 'Learn advanced programming concepts', 45.00, 2, 25),
('T-Shirt', 'Comfortable cotton t-shirt', 19.99, 3, 100),
('Smartphone', 'Latest model smartphone', 699.99, 1, 15);

-- Insert sample orders
INSERT INTO orders (user_id, product, amount, status) VALUES
(1, 'Laptop Pro', 999.99, 'completed'),
(1, 'Wireless Mouse', 29.99, 'completed'),
(2, 'Programming Book', 45.00, 'pending'),
(3, 'T-Shirt', 19.99, 'completed'),
(4, 'Smartphone', 699.99, 'processing'),
(2, 'Laptop Pro', 999.99, 'completed');
