import threading
import random
import time
import math
import csv
import json
import sys
import time
import pickle

#import stats as sts
import numpy

from deap import base
from deap import creator
from deap import tools
from deap import algorithms

import eqpy, ga_utils
        
ga_params = None	

def printf(val):
    print(val)
    sys.stdout.flush()

def obj_func(x):
    return 0

def create_list_of_json_strings(list_of_lists, super_delim=";"):
    # create string of ; separated jsonified maps
    res = []
    for l in list_of_lists:
        jmap = {}
        for i,p in enumerate(ga_params):
            jmap[p.name] = l[i]

        jstring = json.dumps(jmap)
        res.append(jstring)

    return (super_delim.join(res))
    
def create_list_of_params(list_of_lists, super_delim=";"):
    # create string of ; separated param strings
    res = []
    for l in list_of_lists:
        jmap = {}
        param_line = ""
        for i,p in enumerate(ga_params):
            param_line += str(p.name) + " = " + str(l[i]) + "\t"

        res.append(param_line)

    return (super_delim.join(res))

def create_fitnesses(params_string):
    """return equivalent length tuple list
    :type params_string: str
    """
    params = params_string.split(";")
    # get length
    res = [(i,) for i in range(len(params))]
    return (res)

def queue_map(obj_func, pops):
    # Note that the obj_func is not used
    # sending data that looks like:
    # [[a,b,c,d],[e,f,g,h],...]
    if not pops:
        return []

    eqpy.OUT_put(create_list_of_params(pops))
    result = eqpy.IN_get()
    
    printf("received result: " + result)
    
    split_result = result.split(';')
       
    # GA Single objective
#    return [(float(x),) for x in split_result]
    
    
    # GA Multi objective
    y = [x for x in split_result]   # list of strings   
    list_of_lists = [a.split(',') for a in y]   #list of list of strings
    tuple_of_tuples = tuple(tuple(x) for x in list_of_lists)
    data = [tuple(float(x) for x in tup) for tup in tuple_of_tuples]  # convert to floats
    
    return data

def make_random_params():
    """
    Performs initial random draw on each parameter
    """

    global ga_params

    draws = []
    for p in ga_params:
        draws.append(p.randomDraw())
        
    
    return normalize_enrollment_probs(draws)
        
def normalize_enrollment_probs(ind):

    # Need to normalize the enrollment probabilities so that all sum to 1
    # First param is the total enrollment per PY
    # Params 1-5 are the enrollment probs
    sum = ind[1] + ind[2] + ind[3] + ind[4] + ind[5]
    ind[1] = ind[1] / sum
    ind[2] = ind[2] / sum
    ind[3] = ind[3] / sum
    ind[4] = ind[4] / sum
    ind[5] = ind[5] / sum
    
    return ind

# Returns a tuple of one individual
def custom_mutate(individual, indpb):
    """
    Mutates the values in list individual with probability indpb
    """
   
    # # Note, if we had some aggregate constraint on the individual
    # # (e.g. individual[1] * individual[2] < 10), we could copy
    # # individual into a temporary list and mutate though until the
    # # constraint was satisfied

    global ga_params
    for i, param in enumerate(ga_params):
        individual[i] = param.mutate(individual[i], mu=0, indpb=indpb)

    return normalize_enrollment_probs(individual),

def cxUniform(ind1, ind2, indpb):
    c1, c2 = tools.cxUniform(ind1, ind2, indpb)
    return (c1, c2)

def timestamp(scores):
    return str(time.time())


def run():
    
    """
    :param num_iter: number of generations
    :param num_pop: size of population
    :param seed: random seed
    :param strategy: one of 'simple', 'mu_plus_lambda'
    :param ga parameters file name: ga parameters file name (e.g., "ga_params.json")
    :param param_file: name of file containing initial parameters
    """
    eqpy.OUT_put("Params")
    params = eqpy.IN_get()
    
    # parse params
    printf("Parameters: {}".format(params))
    (num_iter, num_pop, seed, mut_prob, ga_params_file) = eval('{}'.format(params))
    
    random.seed(seed)
    global ga_params
    ga_params = ga_utils.create_parameters(ga_params_file)

    creator.create("FitnessMin", base.Fitness, weights=(-1.0,-1.0))
    creator.create("Individual", list, fitness=creator.FitnessMin)
    toolbox = base.Toolbox()
    
    toolbox.register("individual", tools.initIterate, creator.Individual,
                     make_random_params)

    toolbox.register("population", tools.initRepeat, list, toolbox.individual)
    toolbox.register("evaluate", obj_func)
    toolbox.register("mate", cxUniform, indpb=0.5)
    
    mutate_indpb = mut_prob
    toolbox.register("mutate", custom_mutate, indpb=mutate_indpb)
    toolbox.register("select", tools.selNSGA2)
    toolbox.register("map", queue_map)

    pop = toolbox.population(n=num_pop)

    hof = tools.ParetoFront()
#    hof = tools.HallOfFame(1)
    
    stats = tools.Statistics(lambda ind: ind.fitness.values)
    stats.register("avg", numpy.mean, axis=0)
    stats.register("std", numpy.std, axis=0)
    stats.register("min", numpy.min, axis=0)
    stats.register("max", numpy.max, axis=0)
    stats.register("ts", timestamp)

    logbook = tools.Logbook()
    logbook.header = "gen", "evals", "std", "min", "avg", "max"
   
    NGEN = num_iter + 1
    CXPB = 0.5
    
    start_time = time.time()

    # Evaluate the individuals with an invalid fitness
    invalid_ind = [ind for ind in pop if not ind.fitness.valid]
    fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
    for ind, fit in zip(invalid_ind, fitnesses):
        ind.fitness.values = fit

    # This is just to assign the crowding distance to the individuals
    # no actual selection is done
    pop = toolbox.select(pop, len(pop))
    
    hof.update(pop)
    record = stats.compile(pop)
    logbook.record(gen=0, evals=len(invalid_ind), **record)

    hof_out_all = ""
    hof_out_params_all = ""

    # Begin the generational process
    for gen in range(1, NGEN):
        # Vary the population
        offspring = tools.selTournamentDCD(pop, len(pop))
        offspring = [toolbox.clone(ind) for ind in offspring]
        
        for ind1, ind2 in zip(offspring[::2], offspring[1::2]):
            if random.random() <= CXPB:
                toolbox.mate(ind1, ind2)
            
            toolbox.mutate(ind1)
            toolbox.mutate(ind2)
            del ind1.fitness.values, ind2.fitness.values
        
        # Evaluate the individuals with an invalid fitness
        invalid_ind = [ind for ind in offspring if not ind.fitness.valid]
        fitnesses = toolbox.map(toolbox.evaluate, invalid_ind)
        for ind, fit in zip(invalid_ind, fitnesses):
            ind.fitness.values = fit

        # Select the next generation population
        pop = toolbox.select(pop + offspring, num_pop)
        hof.update(pop)
        record = stats.compile(pop)
        logbook.record(gen=gen, evals=len(invalid_ind), **record)
         
        hof_out = str(gen) + "\t" + ';'.join([str(p.fitness.values) for p in hof]) + "\n"
        hof_out_params = str(gen) + "\t" + create_list_of_params(hof) + "\n"
        
        hof_out_all += hof_out
        hof_out_params_all += hof_out_params
        
        # log GA info to experiment folder on the fly (append)
        with open("./pareto.txt", "a") as myfile:
            myfile.write(hof_out)
            
        with open("./pareto_params.txt", "a") as myfile:
            myfile.write(hof_out_params)
        
        # Write log (full overwrite)    
        with open("./stats_log.txt", "w") as myfile:
            myfile.write(str(logbook))

        
    end_time = time.time()

    fitnesses = [str(p.fitness.values) for p in pop]
    hof_out =  [str(p.fitness.values) for p in hof]
 
    eqpy.OUT_put("DONE")
    # return the final population
    eqpy.OUT_put("{}\n{}\n{}\n{}\n{}\n{}\n{}".format(create_list_of_params(pop), ';'.join(fitnesses),
        create_list_of_params(hof), ';'.join(hof_out), logbook,
        hof_out_params_all, hof_out_all))
