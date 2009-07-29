<?php

 /**
  * tWSDL Class
  *
  * The tWSDL Class is an application for outputting a tWSDL file based on the PHPDOC information gathered on an application that extends an fSoapApplication
  *
  * @name tWSDL
  * @author Dallas Gutauckis <dgutauckis@myyearbook.com>
  * @since 2007-07-20
  * @uses fXMLApplication
  * @final
  *
  */

final class tWSDLGenerator extends fXMLApplication {

  function __construct()
  {
//    parent::__construct('application/wsdl+xml');
    parent::__construct();

  }

  function execute($values)
  {
    if ( count($values) === 0)
      throw new fHTTPException(404,'No Class Specified');

    // Build our WSDL and add the output to the fOutput buffer
    $this->build($values[0]);
    $this->output();

    // We're done
    return true;
  }

  /**
   * Build a WSDL Document
   *
   * @param string $classStr
   * @return string
   */
  function build($classStr)
  {

    // Sets our reflection object so we can get methods, properties, and such
    $classObj = new ReflectionClass($classStr);
    $classInfo = new fClassInfo($classStr);

    if ( false === $classObj->isSubclassOf('tSoapApplication' ) )
      throw new fHTTPException(501, 'SOAP Application \'' . $classObj->name . '\' does not exist.');

    // Copy the public dom instance for manipulation
    $dom = $this->dom;

    // Create our root node (definitions)
    // Example: <definitions name="BNQuoteService" targetNamespace="http://www.xmethods.net/sd/BNQuoteService.wsdl" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:soap="http://schemas.xmlsoap.org/wsdl/soap/" xmlns="http://schemas.xmlsoap.org/wsdl/" xmlns:tns="http://www.xmethods.net/sd/BNQuoteService.wsdl" >
    $definitions = $dom->createElement('definitions');
    $definitions->setAttribute('name', $classStr . "Service");
    $definitions->setAttribute('xmlns:xsd', 'http://www.w3.org/2001/XMLSchema');
    $definitions->setAttribute('xmlns:soap', 'http://schemas.xmlsoap.org/wsdl/soap/');
    $definitions->setAttribute('xmlns', 'http://schemas.xmlsoap.org/wsdl/');

    $service = $dom->createElement('service');
    $service->setAttribute('name', $classStr . "Service");

    // If our class has a description... put it in the wsdl document!
    if ( $classInfo->desc != '' )
    {
      $documentation = $dom->createElement('documentation',$classInfo->desc);
      $definitions->appendChild($documentation);
    }

    $port = $dom->createElement('port');
    $port->setAttribute('name', $classStr . "Port");
    $port->setAttribute('binding', 'tns:' . $classStr . "Binding");

    // Create the soapAddressnode
    $soapAddress = $dom->createElement('soap:address');

    // Get the URI for the tSoapServer Class and set the soapAddress
    $ssuri = fSiteProcessor::returnClassURI('tSoapServer');
    $soapAddress->setAttribute('location', $this->fMain->configuration->site->URL . $ssuri . "/" . $classStr);

    $port->appendChild($soapAddress);
    $service->appendChild($port);

    // Get the methods of $classObj class
    $methodsArr = $classObj->getMethods();

    // If we have methods other than "execute"
    if ( count($methodsArr) > 0 )
    {
      // Create our binding node
      $binding = $dom->createElement('binding');
      $binding->setAttribute( 'name', $classStr . 'Binding' );
      $binding->setAttribute('type', $classStr . 'PortType');

      // Create our portType node
      $portType = $dom->createElement('portType');
      $portType->setAttribute('name', $classStr . 'PortType');

      // Create the soap:binding node
      // Example <soap:binding style="rpc" transport="http://schemas.xmlsoap.org/soap/http"/>
      $soapBinding = $dom->createElement('soap:binding');
      /**
       * @todo: Should we be doing something more dynamic here?
       */
      $soapBinding->setAttribute('style','rpc');
      $soapBinding->setAttribute('transport','http://schemas.xmlsoap.org/soap/http');

      $binding->appendChild($soapBinding);

      // Get all of the params for each method
      foreach ($methodsArr as $method)
      {
        if (true == $method->isConstructor() || $method->getDeclaringClass()->name !== $classStr || $method->name == 'execute')
          continue;

        $paramInfo = new fFunctionInfo($method);
        $return = $paramInfo->return;
        $paramInfo = $paramInfo->param;

        // Set the return type to string if no type is declared by a PHPDOC Comment
        $return->type = ($return->type != '') ? ($return->type) : ('string');

        // Create our message request node
        $messageRequest = $dom->createElement('message');
        // Create our message response (return) node
        $messageResponse = $dom->createElement('message');
        // Create our operation node
        $ptOperation = $dom->createElement('operation');

        // Create our soap:operation node, adding the action to it
        $soapOperation = $dom->createElement('soap:operation');
        /**
         * @todo: Address what this action attribute actually should be
         */
        $soapOperation->setAttribute('action','');

        $bOperation = $dom->createElement('operation');
        $bOperation->setAttribute('name', $method->name);
        // Append the soap:operation action portion to the operation node
        $bOperation->appendChild($soapOperation);

        // Set the name attribute to the name of the method
        $messageRequest->setAttribute('name',$method->name);
        $messageResponse->setAttribute('name',$method->name . 'Response');
        $ptOperation->setAttribute('name', $method->name);

        // Set our message response type
        $part = $dom->createElement('part');
        $part->setAttribute('name','return');
        $part->setAttribute('type', 'xsd:' . $return->type);
        $messageResponse->appendChild($part);

        // portType Operation input node
        $ptInput = $dom->createElement('input');
        $ptInput->setAttribute('name',$method->name);
        $ptInput->setAttribute('message',$method->name);

        // portType Operation output node
        $ptOutput = $dom->createElement('output');
        $ptOutput->setAttribute('name',$method->name . 'Response');
        $ptOutput->setAttribute('message',$method->name . 'Response');

        // Example: <soap:body use="encoded" namespace="urn:xmethods-Temperature" encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" />
        $soapBody = $dom->createElement('soap:body');
        $soapBody->setAttribute('use','encoded');
        $soapBody->setAttribute('namespace','base64');
        $soapBody->setAttribute('encodingStyle','http://schemas.xmlsoap.org/soap/encoding/');

        $oSoapBody = clone $soapBody;

        // binding Operation input node
        $bInput = $dom->createElement('input');
        //$bInput->setAttribute('name',$method->name);
        $bInput->appendChild($oSoapBody);

        // binding Operation output node
        $bOutput = $dom->createElement('output');
        //$bOutput->setAttribute('name',$method->name . 'Response');
        $bOutput->appendChild($soapBody);

        // Get the params and create the appropriate nodes
        $params = $method->getParameters();
        foreach ($params as $param)
        {
          // Set the default param type to string
          $paramInfo[$param->name]->type = ($paramInfo[$param->name]->type != '') ? ($paramInfo[$param->name]->type) : ('string');
          // Create our part element
          $part = $dom->createElement('part');
          // Set our name to the variable name of the param
          $part->setAttribute('name',$param->name);
          // Set the param's type based on information we gathered based on doc comments
          $part->setAttribute('type','xsd:' . $paramInfo[$param->name]->type);

          // Append the part node to the message node
          $messageRequest->appendChild($part);
        }

        // Append our message node to the defintions node
        $definitions->appendChild($messageRequest);
        $definitions->appendChild($messageResponse);

        // Add our binding operation input and output nodes
        $bOperation->appendChild($bInput);
        $bOperation->appendChild($bOutput);

        // Add our porttype operation input and output nodes
        $ptOperation->appendChild($ptInput);
        $ptOperation->appendChild($ptOutput);

        // Append the operation node
        $portType->appendChild($ptOperation);
        $binding->appendChild($bOperation);

      }

      $definitions->appendChild($portType);

      // Append the binding node to the definitions node
      $definitions->appendChild($binding);

      // Append the service node to the definitions node
      $definitions->appendChild($service);
    }

    // Finally, add the definitions node (rood node) to our DOMDocument
    $dom->appendChild($definitions);


    return $dom->saveXML();
  }
}

?>
