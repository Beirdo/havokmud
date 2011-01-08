# vim:ts=4:sw=4:ai:et:si:sts=4
package HavokMud::Login;

use strict;
use warnings;

use base 'Mojolicious::Controller';

sub index {
    my $self = shift;

    my $user = $self->param('user') || '';
    my $pass = $self->param('pass') || '';

    return $self->render unless $self->users->check($user, $pass);

    $self->session(user => $user);
    $self->flash(message => 'Thanks for logging in!');
    $self->redirect_to('protected');
}

sub protected {
    my $self = shift;
    return $self->redirect_to('index') unless $self->session('user');
}

sub logout {
    my $self = shift;
    $self->session(expires => 1);
    $self->redirect_to('index');
}

1;
