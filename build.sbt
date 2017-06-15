organization := "com.intel"

name := "imperative"

version := "0.1"

scalaVersion := "2.11.11"

scalacOptions ++= Seq("-deprecation", "-feature", "-unchecked", "-language:reflectiveCalls", "-Ypatmat-exhaust-depth", "80")

addCompilerPlugin("org.scalamacros" % "paradise" % "2.1.0" cross CrossVersion.full)


// Provide a managed dependency on X if -DXVersion="" is supplied on the command line.
// The following are the default development versions, not the "release" versions.
val defaultVersions = Map(
  "chisel3" -> "3.1-SNAPSHOT",
  "chisel-iotesters" -> "1.2-SNAPSHOT"
  )

libraryDependencies ++= (Seq("chisel3","chisel-iotesters").map {
  dep: String => "edu.berkeley.cs" %% dep % sys.props.getOrElse(dep + "Version", defaultVersions(dep)) })

libraryDependencies ++= Seq( "org.scala-lang.modules"  %% "scala-parser-combinators" % "1.0.4")

//libraryDependencies ++= Seq( "com.storm-enroute" %% "scalameter" % "0.9-SNAPSHOT")

libraryDependencies += "com.intel" %% "reporters" % "0.1"

//testFrameworks += new TestFramework( "org.scalameter.ScalaMeterFramework")

resolvers ++= Seq(
  Resolver.sonatypeRepo("snapshots"),
  Resolver.sonatypeRepo("releases")
)

// Recommendations from http://www.scalatest.org/user_guide/using_scalatest_with_sbt
logBuffered in Test := false

// Disable parallel execution when running te
//  Running tests in parallel on Jenkins currently fails.
parallelExecution in Test := false

// SMB: had problems with fork set to true. SBT would report stack traces
fork in Test := false
//javaOptions in Test += "-Xms1024M"
//javaOptions in Test +=  "-Xss128M"



