// See LICENSE for license details.
organization := "com.intel"

name := "vectoradd"

version := "0.1"

scalaVersion := "2.12.3"

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

// Provide a managed dependency on X if -DXVersion="" is supplied on the command line.
val defaultVersions = Map(
  "chisel3" -> "3.1-SNAPSHOT",
  "chisel-iotesters" -> "1.2-SNAPSHOT"
  )

libraryDependencies ++= (Seq("chisel3","chisel-iotesters").map {
  dep: String => "edu.berkeley.cs" %% dep % sys.props.getOrElse(dep + "Version", defaultVersions(dep)) })

/*
libraryDependencies ++= Seq(
  "org.scalatest" %% "scalatest" % "3.0.1",
  "org.scalacheck" %% "scalacheck" % "1.13.4")
 */

libraryDependencies += "com.intel" %% "testutil" % "1.0"
libraryDependencies += "com.intel" %% "accio" % "0.0"
libraryDependencies += "com.intel" %% "designutils" % "0.1"
libraryDependencies += "com.intel" %% "dataflow-building-blocks" % "0.1"
