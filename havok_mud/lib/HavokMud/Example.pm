package HavokMud::Example;

use strict;
use warnings;

use base 'Mojolicious::Controller';

# This action will render a template
sub welcome {
    my $self = shift;

    # Render template "example/welcome.html.ep" with message
    $self->render(message => 'Welcome to the Mojolicious Web Framework!');
}

1;
