# vim:ts=4:sw=4:ai:et:si:sts=4
package Users;

use strict;
use warnings;

my $USERS = {
    sri    => 'secr3t',
    marcus => 'lulz',
    yko    => 'zeecaptain'
};

sub new { bless {}, shift }

sub check {
    my ($self, $user, $pass) = @_;

    # Success
    return 1 if $USERS->{$user} && $USERS->{$user} eq $pass;

    # Fail
    return;
}

1;

