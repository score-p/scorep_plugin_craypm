/*
 * Copyright (c) 2016, Technische Universität Dresden, Germany
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification, are permitted
 * provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of conditions
 *    and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 *    and the following disclaimer in the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse
 *    or promote products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <cstdint>
#include <cstring>
#include <vector>
#include <map>
#include <thread>
#include <chrono>
#include <atomic>
#include <cassert>

extern "C" {
    #include <scorep/SCOREP_MetricPlugins.h>
    #include "pm_lib.h"
}


/*
 * Some intervals and the resulting sample rate:
 *
 * 1000 ms  : 1 Hz
 * 100 ms   : 10 Hz
 * 10 ms    : 100 Hz
 * 1 ms     : 1000 Hz
 * 100 us   : 1 kHz
 *
 */
static std::chrono::milliseconds INTERVAL(100);

typedef std::int32_t scorep_metric_event_id_t;

static std::map<scorep_metric_event_id_t, std::vector<std::uint64_t>> values;

static uint64_t ( * get_scorep_time )( void );

static int monitoring_mode = 0;

inline bool has_accel()
{
    return monitoring_mode & 2;
}

inline bool has_board()
{
    return monitoring_mode & 1;
}

class measure_thread
{
public:
    measure_thread() : m_run(false), m_thread(NULL) {}

    void stop()
    {
        if ( running() )
        {
            m_run = false;

            m_thread->join();

            delete m_thread;
            m_thread = NULL;
        }
    }

    void start()
    {
        if ( has_board() || has_accel() )
        {
            m_run = true;
            m_thread = new std::thread(&measure_thread::run,this);
        }
    }

    bool running()
    {
        return m_run;
    }

private:

    static void run( measure_thread* thread )
    {
        auto start    = std::chrono::high_resolution_clock::now();
#ifdef ENABLE_FRESHNESS_COUNTER
        auto freshness_first = pm_get_freshness();
#endif
        auto energy_first = pm_get_energy();

        uint64_t accel_energy_first, accel_energy;
        int64_t accel_power;

        if ( has_accel() )
           accel_energy_first = pm_get_accel_energy();

        auto old_freshness = 0;

        for(std::int64_t i = 0; thread->running(); i++)
        {
            auto timestamp = get_scorep_time();

            auto freshness_before = pm_get_freshness();

            if ( freshness_before > old_freshness ) {

                auto power = pm_get_power();
                auto energy = pm_get_energy();

                if ( has_accel() ) {
                    accel_power  = pm_get_accel_power();
                    accel_energy = pm_get_accel_energy();
                }

                auto freshness_after = pm_get_freshness();

#ifdef ENABLE_MEASURETIMER_COUNTER
                auto timestamp_after = get_scorep_time();
#endif

                if(freshness_after == freshness_before )
                {
                    values[PM_NCOUNTERS].push_back(timestamp);
#ifdef ENABLE_MEASURETIMER_COUNTER
                    values[PM_NCOUNTERS+1].push_back(timestamp_after - timestamp);
#endif
                    values[PM_POWER].push_back(power);
                    values[PM_ENERGY].push_back(energy - energy_first);
#ifdef ENABLE_FRESHNESS_COUNTER
                    values[PM_FRESHNESS].push_back(freshness_after - freshness_first);
#endif
                    if ( has_accel() ) {
                    values[PM_ACCEL_POWER].push_back(accel_power);
                    values[PM_ACCEL_ENERGY].push_back(accel_energy - accel_energy_first);
                    }

                    old_freshness = freshness_after;

                }
                else
                {
                    // decreasing i means not to increase it in next loop entry
                    i--;
                    continue;
                }
            }

            std::this_thread::sleep_until(start + (i+1)*INTERVAL);
        }
    }

    std::atomic<bool> m_run;
    std::thread* m_thread;
};

static measure_thread thread;

void start_measure() { thread.start(); }
void stop_measure() { thread.stop(); }

/**
 * This helper function template hides the ugly C-style memory allocation from my eyes, so the don't get burned :-)
 */
template <typename T>
T* allocate_c_memory(std::size_t num_elements)
{
    // using calloc, as it also sets the memory to 0 and has a sane interface
    return (T*) calloc(num_elements, sizeof(T));
}


/**
 * The initializing function should check, whether counting of this plugin is generally
 * available for the current system, whether the user has the right to count, whether
 * an external database is available, and so on. Also it should initialize most of the
 * data structures used from now on.
 *
 * @return Whether the plugin could be initialized correctly (0) or not (!= 0)
 */
int32_t init()
{
    monitoring_mode = pm_init();

    if ( monitoring_mode > 0 )
    {
       values[PM_NCOUNTERS] = std::vector<std::uint64_t>();

#ifdef ENABLE_MEASURETIMER_COUNTER
       values[PM_NCOUNTERS+1] = std::vector<std::uint64_t>();
#endif
    }

    if ( has_board() )
    {
       values[PM_POWER]     = std::vector<std::uint64_t>();
       values[PM_ENERGY]    = std::vector<std::uint64_t>();
#ifdef ENABLE_FRESHNESS_COUNTER
       values[PM_FRESHNESS] = std::vector<std::uint64_t>();
#endif
    }

    if ( has_accel() )
    {
       values[PM_ACCEL_POWER]  = std::vector<std::uint64_t>();
       values[PM_ACCEL_ENERGY] = std::vector<std::uint64_t>();
    }

    return 0;
}

/**
 * The add counter function is used to add counters, which may be called per
 * thread, per process, per host or only once. This depends on the run per variable
 * defined in the info struct. However, this should initialize the counting procedure,
 * but not start it. The returned counter ID is eminent for the further measurement
 * process, since Score-P will use this ID from now on to get results, en- and
 * disable the counting and so on. The plugin has to be aware of this counter and
 * the related counting facility structure.
 *
 * @param event_name            Name of the selected metric (provided by @ get_event_info)
 *
 * @return A unique ID (unique within the plugin) or -1 if adding the counter failed.
 */
scorep_metric_event_id_t add_counter( char * event_name )
{
    if(!thread.running())
        start_measure();

    std::string event(event_name);

    if( event == "pm/power")
        return PM_POWER;

    if( event == "pm/energy")
        return PM_ENERGY;

    if( event == "pm/freshness")
        return PM_FRESHNESS;

    if( event == "pm/accel_power")
        return PM_ACCEL_POWER;

    if( event == "pm/accel_energy")
        return PM_ACCEL_ENERGY;

#ifdef ENABLE_MEASURETIME_COUNTER
    if( event == "pm/measurement time")
        return PM_NCOUNTERS+1;
#endif

    std::cerr << " unknown counter: " << event << std::endl;

    return -1;
}

/**
 * This funtions tells scorep some informations for each channel
 */
SCOREP_Metric_Plugin_MetricProperties* get_event_info( char * event_name )
{
    scorep_metric_event_id_t num_counters = 1;

    if (has_board())
        num_counters += 2;

    if (has_accel())
        num_counters += 2;

#ifdef ENABLE_MEASURETIMER_COUNTER
    num_counters++;
#endif

#ifdef ENABLE_FRESHNESS_COUNTER
    num_counters++;
#endif


    SCOREP_Metric_Plugin_MetricProperties * return_values = allocate_c_memory<SCOREP_Metric_Plugin_MetricProperties>(num_counters);

    scorep_metric_event_id_t k = 0;

    if(has_board())
    {

        return_values[ k ].name        = strdup("pm/energy");
        return_values[ k ].description = NULL;
        return_values[ k ].unit        = strdup("J");
        return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
        return_values[ k ].value_type  = SCOREP_METRIC_VALUE_UINT64;
        return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
        return_values[ k ].exponent    = 0;

        k++;

        return_values[ k ].name        = strdup("pm/power");
        return_values[ k ].description = NULL;
        return_values[ k ].unit        = strdup("W");
        return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
        return_values[ k ].value_type  = SCOREP_METRIC_VALUE_INT64;
        return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
        return_values[ k ].exponent    = 0;

        k++;
    }

#ifdef ENABLE_FRESHNESS_COUNTER
    return_values[ k ].name        = strdup("pm/freshness");
    return_values[ k ].description = NULL;
    return_values[ k ].unit        = NULL;
    return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
    return_values[ k ].value_type  = SCOREP_METRIC_VALUE_INT64;
    return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
    return_values[ k ].exponent    = 0;

    k++;
#endif

    if ( has_accel() )
    {
        return_values[ k ].name        = strdup("pm/accel_energy");
        return_values[ k ].description = NULL;
        return_values[ k ].unit        = strdup("J");
        return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
        return_values[ k ].value_type  = SCOREP_METRIC_VALUE_UINT64;
        return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
        return_values[ k ].exponent    = 0;

        k++;

        return_values[ k ].name        = strdup("pm/accel_power");
        return_values[ k ].description = NULL;
        return_values[ k ].unit        = strdup("W");
        return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
        return_values[ k ].value_type  = SCOREP_METRIC_VALUE_INT64;
        return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
        return_values[ k ].exponent    = 0;

        k++;
    }
#ifdef ENABLE_MEASURETIMER_COUNTER

    return_values[ k ].name        = strdup("pm/measurement time");
    return_values[ k ].description = NULL;
    return_values[ k ].unit        = NULL;
    return_values[ k ].mode        = SCOREP_METRIC_MODE_ABSOLUTE_LAST;
    return_values[ k ].value_type  = SCOREP_METRIC_VALUE_INT64;
    return_values[ k ].base        = SCOREP_METRIC_BASE_DECIMAL;
    return_values[ k ].exponent    = 0;

    k++;
#endif

    return_values[ k ].name      = NULL;

    assert( (k+1) == num_counters );

    return return_values;
}

/**
 * all work is done in the destructors, so this function is empty
 */
void finalize()
{
    pm_close();
}

/**
 * This function returns for each channel all datapoints
 */
uint64_t get_all_values( scorep_metric_event_id_t id, SCOREP_MetricTimeValuePair** time_value_list )
{
    if(thread.running())
        stop_measure();

    std::vector<std::uint64_t>& data = values[id];
    std::vector<std::uint64_t>& time = values[PM_NCOUNTERS];

    // allocate memory
    *time_value_list = allocate_c_memory<SCOREP_MetricTimeValuePair>(data.size());


    for(std::size_t i = 0; i < data.size(); ++i)
    {
        (*time_value_list)[i].timestamp = time[i];
        (*time_value_list)[i].value     = data[i];
    }

    return data.size();
}

/**
 * this functions gets called to set the scorep time measure function
 */
void set_clock( uint64_t ( * clock )( void ) )
{
    get_scorep_time = clock;
}

/**
 * This function get called to give some informations about the plugin to scorep
 */
SCOREP_METRIC_PLUGIN_ENTRY( pm_plugin )
{
    /* Initialize info data (with zero) */
    SCOREP_Metric_Plugin_Info info;
    memset( &info, 0, sizeof( SCOREP_Metric_Plugin_Info ) );

    /* Set up the structure */
    info.plugin_version               = SCOREP_METRIC_PLUGIN_VERSION;
    info.run_per                      = SCOREP_METRIC_PER_HOST;
    info.sync                         = SCOREP_METRIC_ASYNC;
    info.delta_t                      = UINT64_MAX;
    info.initialize                   = init;
    info.finalize                     = finalize;
    info.get_event_info               = get_event_info;
    info.add_counter                  = add_counter;
    info.get_all_values               = get_all_values;
    info.set_clock_function           = set_clock;

    return info;
}
