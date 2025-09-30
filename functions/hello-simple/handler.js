'use strict'

module.exports = async (event, context) => {
  return context
    .status(200)
    .headers({"Content-Type": "application/json"})
    .succeed({
      timestamp: new Date().toISOString(),
      originalEndpoint: "/hello",
      serverless: true
    });
}
