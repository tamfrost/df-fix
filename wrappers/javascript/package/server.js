#!/usr/bin/env node

const express = require('express');
const yargs = require('yargs/yargs');
const { hideBin } = require('yargs/helpers');
const path = require('path');
const { commitHash, buildTime } = require('./buildinfo.json')

const argv = yargs(hideBin(process.argv)).option('port', {
    alias: 'p',
    type: 'number',
    description: 'Port to run the server on',
}).argv;

const app = express();
const port = process.env.PORT || argv.port;

console.log('Process env port:', process.env.PORT);
console.log('Build time:', buildTime);
console.log('Commit hash:', commitHash);
console.log('Starting server on port:', port);

app.use(express.static(path.join(__dirname, 'dist')));

const server = app.listen(port || 0, () => {
    if (!server.address()) {
        console.error('Failed to start the server. Please check the port configuration.');
        process.exit(1);
    }
    const actualPort = server.address().port;
    console.log(`Server is running on http://localhost:${actualPort}`);
});