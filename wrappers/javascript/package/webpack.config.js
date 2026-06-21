const path = require('path');
const TerserPlugin = require('terser-webpack-plugin');
const HtmlWebpackPlugin = require('html-webpack-plugin');
const BundleAnalyzerPlugin = require('webpack-bundle-analyzer').BundleAnalyzerPlugin;

const config = {
    entry: {
        "geofix": './index.js',
        "index": './src/index.js',
        // "test": './src/packagetest.js'
    },
    mode: 'production',
    output: {
        filename: pathData => {
            if (pathData.chunk.name === 'geofix') return '../publish/[name].min.js'
            if (pathData.chunk.name === 'index') return '[name].min.js'
            if (pathData.chunk.name === 'test') return '[name].min.js'
        },
        library: 'geofix',
        libraryTarget: 'umd',
        globalObject: 'this'
    },
    optimization: {
        minimize: true,
        minimizer: [new TerserPlugin()]
    },
    module: {
        rules: [
            {
                test: /\.tsx?$/,
                use: 'ts-loader',
                exclude: /node_modules/,
            },
            {
                test: /\.m?js/,
                resolve: {
                    fullySpecified: false,
                },
            },
            {
                test: /\.(woff|woff2|eot|ttf|svg|png)$/,
                type: 'asset/inline',
            }
        ],
    },
    resolve: {
        extensions: ['.tsx', '.ts', '.js'],
        fallback: { crypto: false, path: false, fs: false, util: false }
    },
    devServer: {
        port: 3334,
        client: {
            overlay: {
                warnings: false,
                errors: true
            }
        },
        static: [
            {
                directory: path.join(__dirname, 'doc'),
                publicPath: '/doc',
            }
        ],
        proxy:  [
            {
                context: ['/api'], // Specify the context for the proxy
                target: 'http://localhost:8181/api',
                changeOrigin: true,
                pathRewrite: { '^/api': '' }
            }
            // Uncomment and add more proxies if needed
            // {
            //     context: ['/websocket'],
            //     target: 'http://localhost:8080',
            //     changeOrigin: true,
            //     ws: true,
            //     pathRewrite: { '^/websocket': '' }
            // }
        ],
    },
    plugins: [
        new HtmlWebpackPlugin({
            chunks: ['index'],
            template: "./src/index.html",
        }),
        // new HtmlWebpackPlugin({
        //     filename: 'test.html',
        //     chunks: ['test'],
        //     template: "./src/packagetest.html",
        // }),
        // new BundleAnalyzerPlugin()
    ],
    devtool: 'eval-cheap-source-map',
}

module.exports = async (env) => {

    if (process.env.WEBPACK_SERVE) {
        if (env.server) {
            config.devtool = 'eval-cheap-source-map';
            config.stats = { warnings: false };

            const { server } = require("./server.js");

            server({}, false)
                .then(message => {
                    console.log(message);
                })
                .catch(message => {
                    console.error(message);
                });
        }
    }
    else {
        config.devtool = 'source-map';
    }

    return config;
}