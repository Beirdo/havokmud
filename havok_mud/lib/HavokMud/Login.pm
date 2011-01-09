# vim:ts=4:sw=4:ai:et:si:sts=4
package HavokMud::Login;

use strict;
use warnings;

use base 'Mojolicious::Controller';

sub index {
    my $self = shift;

    my $email  = $self->param('email') || '';
    my $pass   = $self->param('pass') || '';
    my $remote = $self->req->headers->header('X-Real-IP') || '';

    return $self->render unless $self->users->check($email, $pass, $remote);

    $self->session(user => $email);
    $self->flash(message => 'Thanks for logging in!');
    $self->redirect_to('protected');
}

sub register {
    my $self = shift;

    my $email  = $self->param('email') || '';
    my $pass   = $self->param('pass') || '';
    my $pass2  = $self->param('pass2') || '';
    my $remote = $self->req->headers->header('X-Real-IP') || '';

    return $self->render unless 
            $self->users->register($email, $pass, $pass2, $remote);

    $self->session(user => $email);
    $self->flash(message => 'Thanks for registering!  Please confirm your ' .
                            'email address by following the instructions ' .
                            'sent to you.');
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

sub confirm {
    my $self = shift;

    my $code   = $self->param('code') || '';
    my $remote = $self->req->headers->header('X-Real-IP') || '';

    $self->session(expires => 1);

    if ( $self->users->confirm($code, $remote) ) {
        $self->flash(message => 'Thanks for confirming your email address.  ' .
                                'Please login so you can play.');
    } else {
        $self->flash(message => 'Could not confirm.  Perhaps you should ' .
                                'login and resend the confirmation message.');
    }

    $self->redirect_to('index');
}



1;
