<?php
class tHostname
{
  private $hostname;

  function __construct()
  {
    // Fetch it from cache if we have it
    if ( function_exists('apc_fetch') )
        $this->hostname = apc_fetch('hostname');

    // If we didn't have a valid name in the store
    if ( !strlen($this->hostname) )
    {
      // Shell out and get the hostname
      list($this->hostname) = explode('.', shell_exec('hostname'));

      // Store it in cache if we have it
      if ( function_exists('apc_store') )
        apc_store('hostname', $this->hostname, 86400);
    }
  }

  function get()
  {
    return $this->hostname;
  }

}
?>