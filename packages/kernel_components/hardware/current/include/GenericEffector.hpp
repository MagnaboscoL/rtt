/***************************************************************************
  tag: Peter Soetens  Thu Apr 22 20:40:59 CEST 2004  GenericEffector.hpp 

                        GenericEffector.hpp -  description
                           -------------------
    begin                : Thu April 22 2004
    copyright            : (C) 2004 Peter Soetens
    email                : peter.soetens@mech.kuleuven.ac.be
 
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
 
 
#ifndef GENERIC_EFFECTOR_HPP
#define GENERIC_EFFECTOR_HPP

#include <device_drivers/AnalogOutput.hpp>
#include <device_drivers/DigitalOutput.hpp>

#include <pkgconf/system.h>
#ifdef OROPKG_EXECUTION_PROGRAM_PARSER
#include "execution/TemplateDataSourceFactory.hpp"
#include "execution/TemplateCommandFactory.hpp"
#endif

#include <map>
#include <control_kernel/DataServer.hpp>

/**
 * @file GenericEffector.hpp
 * This file contains a for most applications
 * usefull Effector, which uses the Orocos Device
 * Interfaces to communicate with hardware.
 *
 */
namespace ORO_ControlKernel
{
    using namespace ORO_CoreLib;
#ifdef OROPKG_EXECUTION_PROGRAM_PARSER
    using namespace ORO_Execution;
#endif
    using namespace ORO_DeviceInterface;
    using namespace ORO_DeviceDriver;
    using boost::bind;
    using std::make_pair;
    using std::pair;

    /**
     * @brief A GenericEffector can read integers, doubles and booleans
     * and Channel Values.
     */
    struct GenericOutput
        : public ServedTypes<unsigned int, int, double, bool, std::vector<double> >
    {};

    /**
     * @brief An Effector using the ORO_DeviceInterface devices
     * for accessing hardware devices. Off course, you
     * can also use simulated hardware.
     * @ingroup kcomps kcomp_effector
     */
    template <class Base>
    class GenericEffector
        : public Base
    {
    public:
        typedef GenericOutput OutputDataObject;

        /**
         * Necessary typedefs.
         */
        typedef typename Base::OutputType OutputType;
        /** 
         * @brief Create a Generic Effector component with a maximum number of channels.
         * 
         * @param max_chan The maximum number of channels
         * 
         */
        GenericEffector(int max_chan = 1) 
            :  Base("GenericEffector"),
               max_channels("MaximumChannels","The maximum number of virtual analog channels", max_chan),
               usingChannels(0)
        {
            channels.resize(max_chan, 0 );
            chan_out.resize(max_chan, 0);
        }

        virtual bool componentStartup()
        {
            // We check if the required DataObjects are present
            // they are introduced by the Controller component.
            if (
                std::count_if( a_out.begin(), a_out.end(), bind( &GenericEffector<Base>::lacksAOut, this, _1 ) ) ||
                 ( usingChannels && ! Base::Output::dObj()->Get("ChannelValues", chan_DObj) ) )
                return false;
            return true;
        }

        /**
         * @see KernelInterfaces.hpp class ModuleControlInterface
         */
        virtual void push()      
        {
            /*
             * Acces Analog device drivers and Drives
             */
            std::for_each( a_out.begin(), a_out.end(), bind( &GenericEffector<Base>::write_to_aout, this, _1 ) );

            // gather results.
            chan_DObj->Get( chan_out );

            // writeout.
            for (unsigned int i=0; i < channels.size(); ++i)
                if ( channels[i] )
                    channels[i]->value( chan_out[i] );
        }

        /** 
         * @brief Add an AnalogOutput which reads from an Output DataObject.
         * 
         * @param name    The name of the DataObject to read.
         * @param output  The Analog Device to write to.
         * @param channel The channel of the Device to write to.
         * 
         * @return true on success, false otherwise 
         */
        bool addAnalogOutput( const std::string& name, AnalogOutInterface<unsigned int>* output, int channel )
        {
            if ( a_out.count(name) != 0 || kernel()->isRunning() )
                return false;

            // we will fill in the dataobject pointer in componentStartup()
            DataObjectInterface<double>* tmp = 0;
            a_out[name] = make_pair( new AnalogOutput<unsigned int>( output, channel ), tmp);

            return true;
        }

        /** 
         * @brief Remove a previously added AnalogOutput
         * 
         * @param name The name of the DataObject to which it was connected
         * 
         * @return true on success, false otherwise
         */
        bool removeAnalogOutput( const std::string& name )
        {
            if ( a_out.count(name) != 1 || kernel()->isRunning() )
                return false;

            delete a_out[name].first;

            a_out.erase(name);
            return true;
        }

        /** 
         * @brief Add a virtual channel for reading an analog value from the OutputDataObject
         * 
         * @param virt_channel The Channel number of the DataObject.
         * @param output       The Device to write the data to.
         * @param channel      The channel of the device to use.
         * 
         * @return true on success, false otherwise.
         */
        bool addChannel( int virt_channel, AnalogOutInterface<unsigned int>* output, int channel )
        {
            if ( channels[virt_channel] != 0 || kernel()->isRunning() )
                return false;

            channels[virt_channel] = new AnalogOutput<unsigned int>( output, channel );
            ++usingChannels;
            return true;
        }

        /** 
         * @brief Remove the use of a virtual channel
         * 
         * @param virt_channel The number of the channel to remove
         * 
         */
        bool removeChannel( int virt_channel )
        {
            if ( channels[virt_channel] == 0 || kernel()->isRunning() )
                return false;
            
            delete channels[virt_channel];
            --usingChannels;
            return true;
        }


        /** 
         * @brief Add a DigitalOutput.
         * 
         * @param name    The name of the DigitalOutput.
         * @param output  The Device to write to.
         * @param channel The channel/bit of the device to use
         * @param invert  Invert the output or not.
         * 
         */
        bool addDigitalOutput( const std::string& name, DigitalOutInterface* output, int channel, bool invert=false)
        {
            if ( d_out.count(name) != 0 || kernel()->isRunning() )
                return false;

            d_out[name] = new DigitalOutput( output, channel, invert );

            return true;
        }

        /** 
         * @brief Remove a previously added DigitalOutput
         * 
         * @param name The name of the DigitalOutput
         * 
         */
        bool removeDigitalOutput( const std::string& name )
        {
            if ( d_out.count(name) != 1 || kernel()->isRunning() )
                return false;

            delete d_out[name];

            d_out.erase(name);
            return true;
        }

        /**
         * @name Generic Effector Commands
         * Runtime Commands to inspect and set the state of the 
         * Digital Outputs
         * @{
         */
        /** 
         * @brief Switch on a DigitalOutput.
         * 
         * @param name The name of the output to switch on.
         */
        void switchOn( const std::string& name )
        {
            DOutMap::iterator it = d_out.find(name);
            if ( it == d_out.end() )
                return;
            it->second->switchOn();
        }

        /** 
         * @brief Is a DigitalOutput on ?
         * 
         * @param name The DigitalOutput to inspect.
         * 
         * @return true if on, false otherwise.
         */
        bool isOn( const std::string& name ) const
        {
            DOutMap::const_iterator it = d_out.find(name);
            if ( it != d_out.end() )
                return it->second->isOn();
            return false;
        }

        /** 
         * @brief Switch off a DigitalOutput.
         * 
         * @param name The name of the output to switch off.
         */
        void switchOff( const std::string& name )
        {
            DOutMap::const_iterator it = d_out.find(name);
            if ( it == d_out.end() )
                return;
            it->second->switchOff();
        }

        /** 
         * @brief Return the value of an AnalogOutput
         * 
         * @param name The name of the AnalogOutput
         * 
         * @return The physical value.
         */
        double value(const std::string& name) const
        {
            AOutMap::const_iterator it = a_out.find(name);
            if ( it == a_out.end() )
                return 0;
            return it->second->value();
        }

        /** 
         * @brief Return the raw sensor value of an AnalogOutput
         * 
         * @param name The name of the AnalogOutput
         * 
         * @return The raw value.
         */
        unsigned int rawValue(const std::string& name) const
        {
            AOutMap::const_iterator it = a_out.find(name);
            if ( it == a_out.end() )
                return 0;
            return it->second->rawValue();
        }

        /**
         * @}
         */
        
    protected:
#ifdef OROPKG_EXECUTION_PROGRAM_PARSER

        DataSourceFactoryInterface* createDataSourceFactory()
        {
            TemplateDataSourceFactory< GenericEffector<Base> >* ret =
                newDataSourceFactory( this );
            ret->add( "isOn", 
                      data( &GenericEffector<Base>::isOn,
                            "Inspect the status of a Digital Output.",
                            "Name", "The Name of the Digital Output."
                            ) );
            ret->add( "value", 
                      data( &GenericEffector<Base>::value,
                            "Inspect the value of an Analog Output.",
                            "Name", "The Name of the Analog Output."
                            ) );
            ret->add( "rawValue", 
                      data( &GenericEffector<Base>::rawValue,
                            "Inspect the raw value of an Analog Output.",
                            "Name", "The Name of the Analog Output."
                            ) );
            return ret;
        }

        template< class T >
        bool true_gen() const { return true; }

        CommandFactoryInterface* createCommandFactory()
        {
            TemplateCommandFactory< GenericEffector<Base> >* ret =
                newCommandFactory( this );
            ret->add( "switchOn",
                      command( &GenericEffector<Base>::switchOn,
                               &GenericEffector<Base>::true_gen,
                               "Switch A Digital Output on",
                               "Name","The Name of the DigitalOutput."
                               ) ); 
            ret->add( "switchOff",
                      command( &GenericEffector<Base>::switchOff,
                               &GenericEffector<Base>::true_gen,
                               "Switch A Digital Output off",
                               "Name","The Name of the DigitalOutput."
                               ) ); 
            return ret;
        }

#endif

        /**
         * Write to Data to digital output.
         */
        void write_to_aout( pair<std::string, pair<AnalogOutput<unsigned int>*, DataObjectInterface<double>* > > dd )
        {
            dd.second.first->value( dd.second.second->Get() );
        }

        /**
         * Check if the Output DataObject lacks a user requested output.
         */
        bool lacksAOut( pair<std::string,pair<AnalogOutput<unsigned int>*, DataObjectInterface<double>* > > dd )
        {
            // fill in the dataobject, if it fails, abort startup...
            if ( !Base::Output::dObj()->Get(dd.first, dd.second.second) )
                return true; // signals it went wrong
            return false;
        }
            
        Property<int> max_channels;

        std::vector< AnalogOutput<unsigned int>* > channels;
        std::vector<double> chan_out;
        DataObjectInterface< std::vector<double> >* chan_DObj;

        typedef std::map<std::string, DigitalOutput* > DOutMap;
        DOutMap d_out;
        typedef std::map<std::string, pair<AnalogOutput<unsigned int>*, DataObjectInterface<double>* > > AOutMap;
        AOutMap a_out;

        int usingChannels;
    };

}
#endif

