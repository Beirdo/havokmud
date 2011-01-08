# vim:ts=4:sw=4:ai:et:si:sts=4
package Users;

use strict;
use warnings;

use Mojo::Client;
use Mojo::JSON;

sub new { bless {}, shift }

sub check {
    my ($self, $email, $pass) = @_;

    return 0 if (!$email || !$pass);

    my $client  = Mojo::Client->new;
    my $json    = Mojo::JSON->new;
    my $msghash = {email => $email, passwd => $pass};
    my $message = {q => $json->encode($msghash)};
    my $posturl = "http://127.0.0.1:8008/login";
    my $tx = $client->post_form($posturl => $message);
    if (my $res = $tx->success) {
        return 0 if (!$res->json->{success});

        $self->{details} = $res->json->{details};
        # Return success/fail based on response
        return 1;
    }

    # Fail
    return 0;
}

1;

