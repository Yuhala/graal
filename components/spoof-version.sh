#!/usr/bin/env bash
#
# Copyright (c) 2020, Oracle and/or its affiliates. All rights reserved.
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# The Universal Permissive License (UPL), Version 1.0
#
# Subject to the condition set forth below, permission is hereby granted to any
# person obtaining a copy of this software, associated documentation and/or
# data (collectively the "Software"), free of charge and under any and all
# copyright rights in the Software, and any and all patent rights owned or
# freely licensable by each licensor hereunder covering either (i) the
# unmodified Software as contributed to or provided by such licensor, or (ii)
# the Larger Works (as defined below), to deal in both
#
# (a) the Software, and
#
# (b) any piece of software and/or hardware listed in the lrgrwrks.txt file if
# one is included with the Software each a "Larger Work" to which the Software
# is contributed by such licensors),
#
# without restriction, including without limitation the rights to copy, create
# derivative works of, display, perform, and distribute the Software and make,
# use, sell, offer for sale, import, export, have made, and have sold the
# Software and the Larger Work(s), and to sublicense the foregoing rights on
# either these or other terms.
#
# This license is subject to the following condition:
#
# The above copyright notice and either this complete permission notice or at a
# minimum a reference to the UPL must be included in all copies or substantial
# portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

#declare -r JAVA_VERSION="${1:?First argument must be java version.}"
#declare -r GRAALVM_VERSION="${2:?Second argument must be GraalVM version.}"

declare -r JAVA_VERSION="11.0.14"
declare -r GRAALVM_VERSION="22.1.0-dev"

#
# Enter the name of the component jar you want 
# to make installable in the above graalvm version.
#
component_name="ruby-component.jar"
component_id="ruby"
bundle_name="Truffle Ruby"
bundle_symbolic_name="com.oracle.truffle.ruby"

if [[ $JAVA_VERSION == 1.8* ]]; then
    JRE="jre/"
else
	JRE=""
fi
readonly COMPONENT_DIR="component_temp_dir"
readonly LANGUAGE_PATH="$COMPONENT_DIR/$JRE/languages/$component_id"



rm -rf COMPONENT_DIR

mkdir -p "$LANGUAGE_PATH"
cp $component_name "$LANGUAGE_PATH"

#mkdir -p "$LANGUAGE_PATH/launcher"
#cp ../launcher/target/sl-launcher.jar "$LANGUAGE_PATH/launcher/"

mkdir -p "$LANGUAGE_PATH/bin"
#cp ../sl $LANGUAGE_PATH/bin/
#
# Peterson Yuhala
# This is a "nasty" workaround: TODO build real secl native img
#
touch "$LANGUAGE_PATH/bin/$component_id"

#if [[ $INCLUDE_SLNATIVE = "TRUE" ]]; then
#    cp ../native/slnative $LANGUAGE_PATH/bin/
#fi

touch "$LANGUAGE_PATH/native-image.properties"


mkdir -p "$COMPONENT_DIR/META-INF"
{
    echo "Bundle-Name: $bundle_name";
    echo "Bundle-Symbolic-Name: $bundle_symbolic_name";
    echo "Bundle-Version: $GRAALVM_VERSION";
    echo "Bundle-RequireCapability: org.graalvm; filter:=\"(&(graalvm_version=$GRAALVM_VERSION)(os_arch=amd64))\"";
    echo "x-GraalVM-Polyglot-Part: True"
} > "$COMPONENT_DIR/META-INF/MANIFEST.MF"

(
cd $COMPONENT_DIR || exit 1
$JAVA_HOME/bin/jar cfm ../$component_name META-INF/MANIFEST.MF .

echo "bin/$component_id = ../$JRE/languages/$component_id/bin/$component_id" > META-INF/symlinks
#if [[ $INCLUDE_SLNATIVE = "TRUE" ]]; then
#    echo "bin/slnative = ../$JRE/languages/sl/bin/slnative" >> META-INF/symlinks
#fi
$JAVA_HOME/bin/jar uf ../$component_name META-INF/symlinks

{
    echo "$JRE"'languages/ruby/bin/ruby = rwxrwxr-x'
    #echo "$JRE"'languages/sl/bin/slnative = rwxrwxr-x'
} > META-INF/permissions
$JAVA_HOME/bin/jar uf ../$component_name META-INF/permissions
)
rm -rf $COMPONENT_DIR
