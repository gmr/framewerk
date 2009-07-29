<?php
  class tRequestPassthrough extends fApplication
  {
    function execute()
    {
      // Get our URL for the destination
      $destination = $this->uriValues[0];
      $registry = new fRegistry($this);
      $url = strval($registry->$destination);
      if ( !strlen($url) ) throw new fHTTPException(404, 'Request destination not found.');

      if ( get_magic_quotes_gpc( ) && is_array( $_POST ) )
      {
        foreach ( $_POST as $key=>$val )
        {
          if ( is_string( $val ) )
          {
            $_POST[ $key ] = stripslashes( $val );
          }
        }
      }

      // Initalize curl
      $curl = curl_init();

      // Set the request type if it's not GET
      if ( $_SERVER["REQUEST_METHOD"] == 'POST' )
      {
        curl_setopt($curl, CURLOPT_POST, true);
      } elseif ( $_SERVER["REQUEST_METHOD"] == 'PUT' )  {
        curl_setopt($curl, CURLOPT_PUT, true);
      }

      // Header array
      $headers = array();

      // Cookies
      $headers[] = 'COOKIE: ' . $_SERVER["HTTP_COOKIE"] . ';';

      // User Agent
      if ( isset($_SERVER['HTTP_USER_AGENT']) ) $headers[] = 'User-Agent: ' . $_SERVER['HTTP_USER_AGENT'];

      // User Agent
      if ( isset($_SERVER['HTTP_REFERER']) ) $headers[] = 'Referer: ' . $_SERVER['HTTP_REFERER'];

      // Accept
      if ( isset($_SERVER['HTTP_ACCEPT']) ) $headers[] = 'Accept: ' . $_SERVER['HTTP_ACCEPT'];

      // Language
      if ( isset($_SERVER['HTTP_ACCEPT_LANGUAGE']) ) $headers[] = 'Accept-Language: ' . $_SERVER['HTTP_ACCEPT_LANGUAGE'];

      // Encoding
      if ( isset($_SERVER['HTTP_ACCEPT_ENCODING']) ) $headers[] = 'Accept-Encoding: ' . $_SERVER['HTTP_ACCEPT_ENCODING'];

      // Add the headers
      curl_setopt($curl, CURLOPT_HTTPHEADER, $headers);

      // Timeouts
      curl_setopt($curl, CURLOPT_TIMEOUT, 10);

      // Set the URL w/ GET Params if it's a get pass through
      if ( ( $_SERVER["REQUEST_METHOD"] == 'GET' ) && ( strlen($_SERVER['QUERY_STRING']) ) )
        $url .= '?' . $_SERVER['QUERY_STRING'];

      // Set the POSTFIELDS if we have postfields
      if ( ( $_SERVER["REQUEST_METHOD"] == 'POST' ) && ( count($_POST) ) )
        curl_setopt($curl, CURLOPT_POSTFIELDS, $_POST);

      // Build and make the request
      curl_setopt($curl, CURLOPT_URL, $url);
      curl_setopt($curl, CURLOPT_HEADER, false);
      //curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
      $data = curl_exec($curl);
      curl_close($curl);
      exit();

      if ( !$data )
        throw new fHTTPException(501, 'Did not return any data from request.');

      // Remove carriage returns
      $data = str_replace("\r", '', $data);

      // Break out the header from the data
      $temp = explode("\n\n", $data, 2);

      // Output headers
      $header = explode("\n", $temp[0]);

      foreach ( $header AS $headerRow )
        if ( strstr($headerRow, 'Content-Type') )
          Header(trim($headerRow));

      // Output the content
      echo $temp[1];
      exit();
    }
  }
?>
