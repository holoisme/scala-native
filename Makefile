
build:
	sbt "set scalaVersion := \"3.8.3\"; project nativelib3" publishLocal
	rm -rf "./nativelib/.2.12/target/scala-2.12/"
	sbt "set scalaVersion := \"3.8.3\"; project toolsJVM2_12" publishLocal

buildtools:
	rm -rf "./nativelib/.2.12/target/scala-2.12/"
	sbt "set scalaVersion := \"3.8.3\"; project toolsJVM2_12" publishLocal

buildnative:
	sbt "set scalaVersion := \"3.8.3\"; project nativelib3" publishLocal
