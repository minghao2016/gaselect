% Generated by roxygen2: do not edit by hand
% Please edit documentation in R/formatSegmentation.R
\name{formatSegmentation}
\alias{formatSegmentation}
\alias{formatSegmentation,GenAlgPLSEvaluator,list-method}
\alias{formatSegmentation,GenAlgUserEvaluator,list-method}
\alias{formatSegmentation,GenAlgLMEvaluator,list-method}
\alias{formatSegmentation,GenAlgFitEvaluator,list-method}
\title{Format the raw segmentation list returned from the C++ code into a usable list}
\usage{
formatSegmentation(object, segments)

\S4method{formatSegmentation}{GenAlgPLSEvaluator,list}(object, segments)

\S4method{formatSegmentation}{GenAlgUserEvaluator,list}(object, segments)

\S4method{formatSegmentation}{GenAlgLMEvaluator,list}(object, segments)

\S4method{formatSegmentation}{GenAlgFitEvaluator,list}(object, segments)
}
\arguments{
\item{object}{The Evaluator object.}

\item{segments}{The raw segmentation list.}
}
\value{
A list of the form replication -> outerSegment -> (calibration, validation, inner -> (test, train))
}
\description{
Format the raw segmentation list returned from the C++ code into a usable list
}
