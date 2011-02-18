#!/usr/bin/php -q
<?php

chdir(dirname(__FILE__));
if ( $_SERVER['argc'] < 2 )
{
  die("No arguments provided.\n");
}

if ( isset( $_ENV['SVC'] ) && file_exists( 'Configuration/fRegistry/Services.xml.production' ) )
{
  $xml = simplexml_load_file( '../Common/Configuration/fRegistry/Services.xml.production' );
  if ( ! $xml )
  {
    die("Unable to lookup service configuration!\n");
  }
  $svc = $xml->xpath( '//node[@name="Services"]/instance[name="'.$_ENV['SVC'].'"]');
  unset( $xml );
  if ( ! $svc || count( $svc ) == 0 )
  {
    die( "Unable to find configuration for service named '" . $_ENV['SVC'] . "'.\n" );
  }
  $_SERVER['SERVER_NAME'] = parse_url( $svc[0]->url, PHP_URL_HOST );
  unset( $svc );
} else if ( isset( $_ENV['SVC_HOST'] ) )
{
  $_SERVER['SERVER_NAME'] = $_ENV['SVC_HOST'];
}

$_SERVER['REDIRECT_URL'] = $_SERVER['argv'][1];

try
{
  include('index.php');
} catch ( Exception $e )
{
  fwrite(STDERR, print_r($e, true));
}
