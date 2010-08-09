// -*- C++ -*-
//
// $Id$

// ****  Code generated by the The ACE ORB (TAO) IDL Compiler ****
// TAO and the TAO IDL Compiler have been developed by:
//       Center for Distributed Object Computing
//       Washington University
//       St. Louis, MO
//       USA
//       http://www.cs.wustl.edu/~schmidt/doc-center.html
// and
//       Distributed Object Computing Laboratory
//       University of California at Irvine
//       Irvine, CA
//       USA
//       http://doc.ece.uci.edu/
// and
//       Institute for Software Integrated Systems
//       Vanderbilt University
//       Nashville, TN
//       USA
//       http://www.isis.vanderbilt.edu/
//
// Information about TAO is available at:
//     http://www.cs.wustl.edu/~schmidt/TAO.html

// TAO_IDL - Generated from 
// ../../../ACE_wrappers/TAO/TAO_IDL/be/be_codegen.cpp:1196

#include "ServiceI.h"

using namespace RTT;
using namespace RTT::detail;

// Implementation skeleton constructor
RTT_corba_CService_i::RTT_corba_CService_i ( RTT::interface::Service::shared_ptr service, PortableServer::POA_ptr poa)
    : RTT_corba_CConfigurationInterface_i( service.get(), PortableServer::POA::_duplicate( poa) ), 
      RTT_corba_COperationRepository_i( service.get(), PortableServer::POA::_duplicate( poa) ),
      mpoa(poa), mservice(service)
{
}

// Implementation skeleton destructor
RTT_corba_CService_i::~RTT_corba_CService_i (void)
{
}

PortableServer::POA_ptr RTT_corba_CService_i::_default_POA()
{
    return PortableServer::POA::_duplicate(mpoa);
}

char * RTT_corba_CService_i::getName (
    void)
{
    return CORBA::string_dup( mservice->getName().c_str() );
}

char * RTT_corba_CService_i::getServiceDescription (
    void)
{
    return CORBA::string_dup( mservice->doc().c_str() );
}

::RTT::corba::CService::CProviderNames * RTT_corba_CService_i::getProviderNames (
    void)
{
    Service::ProviderNames names = mservice->getProviderNames();
    ::RTT::corba::CService::CProviderNames_var result = new ::RTT::corba::CService::CProviderNames();
    result->length( names.size() );
    for (unsigned int i=0; i != names.size(); ++i )
        result[i] = CORBA::string_dup( names[i].c_str() );

    return result._retn();
}

::RTT::corba::CService_ptr RTT_corba_CService_i::getService (
    const char * service_name)
{
    Service::shared_ptr provider = mservice->getService(service_name);
    if ( !provider )
	return RTT::corba::CService::_nil();
    
    RTT_corba_CService_i* serv_i;
    RTT::corba::CService_var serv;
    serv_i = new RTT_corba_CService_i( provider, mpoa );
    serv = serv_i->activate_this();
    //CService_i::registerServant(serv, mtask->provides(service_name));
    return RTT::corba::CService::_duplicate( serv.in() );
}

::CORBA::Boolean RTT_corba_CService_i::hasService (
    const char * name)
{
    return mservice->hasService( name );
}
