# vim:ts=4:sw=4:ai:et:si:sts=4
package HavokMud;

use strict;
use warnings;

use base 'Mojolicious';

use Users;

# This method will run once at server start
sub startup {
    my $self = shift;

    # Setup logging level
    $self->app->log->level('debug');

    # Setup secret for the signed cookies
    $self->app->secret('This should come from a JSON configfile');

    my $users = Users->new;
    $self->helper(users => sub { return $users });

    # Documentation browser under "/perldoc" (this plugin requires Perl 5.10)
    $self->plugin('pod_renderer');

    # Routes
    my $r = $self->routes;
    $r->any('/')->to('login#index')->name('index');
    $r->get('/protected')->to('login#protected')->name('protected');
    $r->get('/logout')->to('login#logout')->name('logout');
}

1;
