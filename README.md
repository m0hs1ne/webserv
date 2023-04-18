# HTTP Server using kqueue

This is a non-blocking HTTP server implementation that uses the kqueue system call for I/O operations. It is designed to be fast, efficient, and scalable, handling multiple connections simultaneously with minimal resource consumption.

## Features

- Takes a configuration file as an argument or uses a default path
- Non-blocking and uses kqueue for all I/O operations between the client and the server (listening included)
- kqueue checks read and write at the same time
- No read or write operation without going through kqueue
- Compatible with web browsers
- Accurate HTTP response status codes
- Default error pages if none provided
- Serves a fully static website
- Clients can upload files
- Supports GET, POST, and DELETE methods
- Stress-tested and stays available at all costs
- Able to listen to multiple ports (configured in the configuration file)

## Installation

To install the server, clone the repository and run:

```
make
```

This will compile the server binary, which can then be run with a configuration file like this:

```
./webserv conf/file.conf
```

## Configuration

The web server is configured using a configuration file that specifies server settings and location blocks. Here is an example of a configuration file:

```server {
	listen 8081 localhost;
	server_name localhost;
	include /Users/mel-hada/Desktop/webserv/conf/mime.types;

	error_page 404 /Users/mel-hada/Desktop/webserv/Content/error/404.html;

	root /Users/mel-hada/Desktop/webserv/Content;
	location / {
		method GET POST DELETE;
		# autoindex on;
		upload_enable on;
		# index /login.html;
		# return 301 s.php;
		cgi_extension py php;
		cgi_path /usr/bin/python3 ./php-cgi;
	}

	location /upload {
		method POST;
		upload_enable on;
		# upload_path /uploads;
		client_max_body_size 2G;
	}
}
```

## Usage

Once the server is running, it can be accessed through a web browser by navigating to the IP address or hostname of the machine it is running on, followed by the port number (e.g. `http://localhost:8080`).

The server will serve any files located in the `root` directory, and will allow clients to upload files to the `upload_path` directory. It supports GET, POST, and DELETE methods for retrieving, creating, and deleting files.
