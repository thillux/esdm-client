#!/usr/bin/env python3
import matplotlib.pyplot as plt
import numpy as np
import math
import json
import natsort
from scipy.stats import chisquare

import os

workingDir = "./build/res/"

benchmarkType = ["measureEntropy", "timeGetRandom", "timeToSeed"]
benchmarkDataString = "data.bench.json"
timeUnitResolution = "nanoseconds"

files = os.listdir(workingDir)
#print(files)

#things to plot:
#tail latencies <-- per run or first run only?
#box-plot rates <-- all runs
#ratio sum(invokationDuration)/totalDuration
#chisquared of output bytes?

def makeGenericPlot(xValues, yValues, xLabel="", yLabel="", title="", filename="defaultPlotName", format="pdf"):
    fig, ax = plt.subplots(figsize=(15,5))
    ax.plot(xValues, yValues, marker = ".", markerfacecolor="black")
    ax.set_xlabel(xLabel)
    ax.set_ylabel(yLabel)
    ax.set_xlim(0, max(xValues))
    # ax.set_ylim(0, max(yValues))

    fig.savefig(filename, format=format)

def tailLatencyPlot(latencies, outputFileName, title = "Cumulative Distribution Function of 'invokationDurations' for 1 run", description = ""):
    x = np.sort(latencies)
    y = np.linspace(1/len(latencies), 1, len(latencies))
    xLabel = f"Latencies in {timeUnitResolution}"
    yLabel = "Cumulative Distribution Function"
    fig, ax = plt.subplots()
    ax.set_xlim(0, max(x))
    ax.set_ylim(0, 1)
    ax.plot(x, y, marker=".", linestyle = "None")
    ax.ticklabel_format( style="scientific")
    ax.set_xlabel(xLabel)
    ax.set_ylabel(yLabel)
    ax.grid(axis="both")
    percentile = 0.95
    xPercentileValue = x[np.where(y > percentile)][0]
    yPercentileValue = y[np.where(y >= percentile)][0]
    # print(f"xPercentileValue:{xPercentileValue}<<------")
    # print(f"yPercentileValue:{yPercentileValue}<<------")
    ax.vlines(xPercentileValue,0,yPercentileValue, linestyle="--", colors="black",label=f"{xPercentileValue}")
    ax.hlines(yPercentileValue,0,xPercentileValue, linestyle="--", colors="black",label=f"{yPercentileValue}")
    ax.text(x=xPercentileValue/(2*max(x)), y= yPercentileValue + 0.01, transform=ax.transAxes, s = f"{int(100*percentile)}%", size = 12, color="black")
    ax.text(x=xPercentileValue/max(x), y= 0.01, horizontalalignment = "left", transform=ax.transAxes, s = f"{xPercentileValue}", size = 12, color="black")
    ax.set_title(title)
    fig.text(0.5, 0, description, ha="center")
    fig.savefig(outputFileName, format="pdf")

def makeBoxPlot(data, outputFileName, xLabel="", yLabel="", totalYScale = False):
    fig, ax = plt.subplots()
    ax.boxplot(data)
    xLabel = xLabel
    yLabel = yLabel
    ax.set_xlabel(xLabel)
    ax.set_ylabel(yLabel)
    if totalYScale:
        ax.set_ylim(0, max(data))  
    ax.grid(axis="both")
    xTicks = ax.get_xticks()
    if len(xTicks) == 1:
        xTicks = []
    ax.set_xticks(xTicks)
    fig.savefig(outputFileName, format="pdf")


def chisquaredOfRawOutputBytes(outputBytesAllRepetitions, expectedTotalBytes, flattenList = True):
    repetitions = len(outputBytesAllRepetitions)
    # print(f"repetitions:{repetitions}")
    requests = min([len(x) for x in outputBytesAllRepetitions])
    # print(f"requests:{requests}")
    #strip '0x' prefix from returnedBytes
    for i in range(len(outputBytesAllRepetitions)):
        outputBytesAllRepetitions[i] = [singleEntry[2:] for singleEntry in outputBytesAllRepetitions[i]]
        # print(outputBytesAllRepetitions[i])
    if flattenList :
        outputBytesAllRepetitions = [singleEntry for encompasedList in outputBytesAllRepetitions for singleEntry in encompasedList]
    # print(f"len(all):{len(outputBytesAllRepetitions)}")
    outputBytesString = ''.join(outputBytesAllRepetitions)
    # print(outputBytesString)
    #outputBytesString should be even, if the benchmark retrieved whole bytes from esdm
    lengthOutputBytesString = len(outputBytesString)
    # print(lengthOutputBytesString/2)
    if lengthOutputBytesString%2 != 0:
        print(f"""number of bytes indicate that no whole bytes were written during the benchmark.
        number of written bytes: {lengthOutputBytesString/float(2)}""")
    if lengthOutputBytesString/2 != expectedTotalBytes:
        print(f"""number of bytes does not match the expected amount.
        actual amount of bytes  :{lengthOutputBytesString/2}
        expected amount of bytes:{expectedTotalBytes}""")
    expectedFrequencies = np.full(256,1/256*expectedTotalBytes)
    # print(expectedFrequencies)
    # print(f"{expectedFrequencies[0]}<<<<---test")

    splitAfterNCharacters = 2
    individualByteList = [outputBytesString[i:i+splitAfterNCharacters] for i in range(0, lengthOutputBytesString,splitAfterNCharacters)]
    # print(individualByteList)
    individualIntList = [int(byteStr, 16) for byteStr in individualByteList]
    # print(individualIntList)
    actualFrequencies = np.bincount(individualIntList, minlength=256)
    # print(actualFrequencies)
    res = chisquare(f_obs=actualFrequencies, f_exp=expectedFrequencies)
    print(f"result of chisquared test statistic:{res.statistic}")
    print(f"result of chisquared test pvalue:{res.pvalue}")

    with open("testOutput", "wb") as outputHandle:
        for element in individualIntList:
            outputHandle.write(element.to_bytes(1, byteorder="big"))

def timeGetRandomEval(outputFiles):
    firstIteration = True
    rateList = []
    ratioList = []
    outputBytesAllRepetitions = []
    requests = 0
    totalBytes = 0
    totalBytesLastRepetition = 0
    savedReturnedValues = False
    for output in outputFiles:
        with open(workingDir + output, "r") as fileHandle:
            fileAsString = fileHandle.read()
            jsonOutput = json.loads(fileAsString)
            rate = float(jsonOutput["data"]["rate"]["bytesPerSeconds"])
            # print(rate)
            rateList.append(rate)
            esdmRpccFunction = jsonOutput["data"]["esdm_rpcc_function"]

            totalDuration = jsonOutput["data"]["duration"][timeUnitResolution]
            # print(totalDuration)
            
            outputBytesCurrentRepetition = []
            invokationDurations = []
            if "returnedValues" in jsonOutput["data"]:
                savedReturnedValues = True
                totalBytesThisRepetition = jsonOutput["data"]["totalBytesReturned"]
                totalBytes += totalBytesThisRepetition
                if firstIteration:
                    totalBytesLastRepetition = totalBytesThisRepetition
                if totalBytesLastRepetition != totalBytesThisRepetition:
                    print(f"""returned bytes between repetitions do not match.
                    #bytesLastRepetition:{totalBytesLastRepetition}
                    #bytesThisRepetition:{totalBytesThisRepetition}""")
                    totalBytesLastRepetition = totalBytesThisRepetition
                returnedValues = jsonOutput["data"]["returnedValues"]
                lengthReturnedValues = len(returnedValues)
                if firstIteration :
                    requests = lengthReturnedValues
                else:
                    if requests != lengthReturnedValues:
                        requests = min(requests, lengthReturnedValues)
                        print(f"Number of request per benchmark repetition is different. choosing min: {requests}")
                # print(returnedValues)
                # print(lengthReturnedValues)
                for i in returnedValues:
                    invokationDurations.append(i["invokationDuration"][timeUnitResolution])
                    outputBytesCurrentRepetition.append(i["returnedBytes"])
                # print(invokationDurations)
                # print(outputBytesCurrentRepetition)
                outputBytesAllRepetitions.append(outputBytesCurrentRepetition)
                sumInvokationDurations = sum(invokationDurations)
                ratioDuration = sumInvokationDurations/totalDuration
                # print(f"ratio sumInvokationDurations/totalDuration:{sumInvokationDurations}/{totalDuration}={ratioDuration}")
                ratioList.append(ratioDuration)
                #only plot tail latency for the first iteration
                description = f"number of measurements:{lengthReturnedValues}"
                if(firstIteration):
                    tailLatencyPlot(invokationDurations, "testTailLatencyPlot",description=description)
                

            # print(totalBytesReturned)
            fileHandle.close()
        firstIteration = False
    # print(outputBytesAllRepetitions)
    # print(rateList)
    xLabelString =  f"timeGetRandom@{len(outputFiles)}repetitions and {requests}requests"
    makeBoxPlot(rateList, "timeGetRandomBoxPlotRate", xLabelString, "rate in bytes per second")
    makeBoxPlot(ratioList, "timeGetRandomBoxPlotRatio", xLabelString, "ratio: (sum of invokation duration)/(total duration) of one repetition")
    chisquaredOfRawOutputBytes(outputBytesAllRepetitions, totalBytes)

def timeToSeedEval(outputFiles):
    timeList = []
    repetitions = len(outputFiles)
    for output in outputFiles:
        with open(workingDir + output , "r") as fileHandle:
            fileAsString = fileHandle.read()
            jsonOutput = json.loads(fileAsString)
            timeToSeedThisRepetition = int(jsonOutput["data"]["measurement"][timeUnitResolution])
            # print(f"{timeToSeedThisRepetition}<----")
            timeList.append(timeToSeedThisRepetition)
            fileHandle.close()
    # print(timeList)
    makeBoxPlot(timeList, "timeToSeedBoxPlot")


def measureEntropyEval(outputFiles):
    repetitions = len(outputFiles)
    currentRepetition = 0
    for output in outputFiles:
        entropyLevelList = []
        timePointList = []
        startTime = 0
        startTimeSet = False
        with open(workingDir + output , "r") as fileHandle:
            fileAsString = fileHandle.read()
            jsonOutput = json.loads(fileAsString)
            # print(jsonOutput)
            jsonMeasurements = jsonOutput["data"]["measurements"]
            numberOfMeasurements = len(jsonMeasurements)
            # print(numberOfMeasurements)
            for entry in jsonMeasurements:
                entropyLevel = entry["entropyLevel"]
                timePoint = entry["timestamp"]
                if not startTimeSet:
                    startTimeSet = True
                    startTime = timePoint
                timePoint = timePoint - startTime
                entropyLevelList.append(entropyLevel)
                timePointList.append(timePoint)
            fileHandle.close()
        makeGenericPlot(timePointList, entropyLevelList, "benchmark-time in nanoseconds", "entropy level in bit")

    

for benchmark in benchmarkType:
    outputFiles = [x for x in files if benchmark in x and benchmarkDataString in x]
    #todorm: remove following line. it is just easier to examine debug output if files are sorted this way
    outputFiles = natsort.natsorted(outputFiles)
    # print(f"{benchmark} files:\n{outputFiles}")
    outputFilesEmpty = len(outputFiles) == 0 
    if outputFilesEmpty:
        continue

    #assumption is that the repetition number is the first string after '{benchmarkType}.' in the file name
    highestRepetitionNumber = int(outputFiles[-1].split(".")[1])
    # print(f"###highestRepetitionNumber: {highestRepetitionNumber}\n")
    countOutputFiles = len(outputFiles)
    if highestRepetitionNumber + 1 != countOutputFiles :
        print(
        f"""Error while reading input Files of {benchmark}:
        Highest repetition number + 1:{highestRepetitionNumber + 1} 
        does not match number of files in directory: {countOutputFiles} """
        )
    # print(f"{benchmark}--->:{countOutputFiles}\n")

    if benchmark == "timeGetRandom":
        timeGetRandomEval(outputFiles)

    if benchmark == "timeToSeed":
        timeToSeedEval(outputFiles)

    if benchmark == "measureEntropy":
        measureEntropyEval(outputFiles)