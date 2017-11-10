// See LICENSE for license details.
organization := "com.intel"
version := "0.0"

name := "accio"

scalaVersion := "2.12.3"

scalacOptions ++= Seq("-deprecation", "-feature", "-unchecked", "-language:reflectiveCalls")

// Provide a managed dependency on X if -DXVersion="" is supplied on the command line.
// The following are the default development versions, not the "release" versions.
val defaultVersions = Map(
  "chisel3" -> "3.1-SNAPSHOT",
  "chisel-iotesters" -> "1.2-SNAPSHOT"
  )

libraryDependencies ++= (Seq("chisel3","chisel-iotesters").map {
  dep: String => "edu.berkeley.cs" %% dep % sys.props.getOrElse(dep + "Version", defaultVersions(dep)) })
  
libraryDependencies += "com.intel" %% "testutil" % "1.0"
libraryDependencies += "com.intel" %% "designutils" % "0.1"

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

parallelExecution in Test := false
