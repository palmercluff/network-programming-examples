#!/usr/bin/perl -w
# Filename : perlServer.pl

use strict;
use Socket;

# initialize host and port

my $num_args = $#ARGV + 1;

if ($num_args != 1){
    print "Usage: name.pl <port #>\n";
    exit;
}

my $port = $ARGV[0];

my $proto = getprotobyname('tcp');
my $server = "localhost";  # Host IP running the server

# create a socket, make it reusable
socket(SOCKET, PF_INET, SOCK_STREAM, $proto)
    or die "Can't open socket $!\n";
setsockopt(SOCKET, SOL_SOCKET, SO_REUSEADDR, 1)
    or die "Can't set socket option to SO_REUSEADDR $!\n";

# bind to a port, then listen
bind( SOCKET, pack_sockaddr_in($port, inet_aton($server)))
    or die "Can't bind to port $port! \n";

listen(SOCKET, 5) or die "listen: $!";
print "SERVER started on port $port\n";

# accepting a connection
my $client_addr;
while ($client_addr = accept(NEW_SOCKET, SOCKET)) {
    # send them a message, close connection
    my $name = gethostbyaddr($client_addr, AF_INET );
    print NEW_SOCKET "Smile from the server";
    print "Connection recieved from $name\n";
    close NEW_SOCKET;
}
