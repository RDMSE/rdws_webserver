-- Production Seed Data
-- Essential data for production environment

-- Insert essential categories
INSERT INTO categories (name, description) VALUES
('Electronics', 'Electronic devices and gadgets'),
('Books', 'Books and educational materials'),
('Clothing', 'Apparel and accessories'),
('Home & Garden', 'Home improvement and garden supplies'),
('Sports', 'Sports equipment and accessories')
ON CONFLICT (name) DO NOTHING;

-- Insert admin user (if needed)
INSERT INTO users (name, email) VALUES
('System Admin', 'admin@rdws.com')
ON CONFLICT (email) DO NOTHING;

-- Note: Additional production data should be managed through application
-- or separate admin tools for security reasons
