package HavokMud;

use strict;
use warnings;

use base 'Mojolicious';

# This method will run once at server start
sub startup {
    my $self = shift;

    # Documentation browser under "/perldoc" (this plugin requires Perl 5.10)
    $self->plugin('pod_renderer');

    # Routes
    my $r = $self->routes;

    # Normal route to controller
    $r->route('/welcome')->to('example#welcome', id => 1);
}

1;
