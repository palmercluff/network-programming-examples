#!/usr/bin/perl -w
# Filename : perlClient.pl

use strict;
use Socket;

# initialize host and port
my $num_args = $#ARGV + 1;

if ($num_args != 1){
    print "Usage: name.pl <port #>\n";
    exit;
}

my $port = $ARGV[0];;
my $server = "localhost";  # Host IP running the server

# create the socket, connect to the port
socket(SOCKET,PF_INET,SOCK_STREAM,(getprotobyname('tcp'))[2])
    or die "Can't create a socket $!\n";
connect( SOCKET, pack_sockaddr_in($port, inet_aton($server)))
    or die "Can't connect to port $port! \n";

my $line;
while ($line = <SOCKET>) {
    print "$line\n";
}

close SOCKET or die "close: $!";
