# vim:ts=4:sw=4:ai:et:si:sts=4
package Users;

use strict;
use warnings;

use Mojo::Client;
use Mojo::JSON;

sub new { bless {}, shift }

sub check {
    my ($self, $email, $pass, $remote) = @_;

    return 0 if (!$email || !$pass);

    my $client  = Mojo::Client->new;
    my $json    = Mojo::JSON->new;
    my $msghash = {email => $email, passwd => $pass, remoteip => $remote};
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

sub register {
    my ($self, $email, $pass, $pass2, $remote) = @_;

    return 0 if (!$email || !$pass || !$pass2 || ($pass ne $pass2));

    my $client  = Mojo::Client->new;
    my $json    = Mojo::JSON->new;
    my $msghash = {email => $email, passwd => $pass, remoteip => $remote};
    my $message = {q => $json->encode($msghash)};
    my $posturl = "http://127.0.0.1:8008/register";
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

sub confirm {
    my ($self, $code, $remote) = @_;

    return 0 if (!$code);

    my $client  = Mojo::Client->new;
    my $json    = Mojo::JSON->new;
    my $msghash = {code => $code, remoteip => $remote};
    my $message = {q => $json->encode($msghash)};
    my $posturl = "http://127.0.0.1:8008/confirm";
    my $tx = $client->post_form($posturl => $message);
    if (my $res = $tx->success) {
        return $res->json->{success};
    }

    # Fail
    return 0;
}

1;

