#!/bin/bash
CDIR=`pwd`
cd /tmp
rm -rf /tmp/maddy /tmp/markdown.1 /tmp/markdown.2 /tmp/markdown.h
git clone https://github.com/progsource/maddy
cat > /tmp/markdown.1 <<EOF
#ifndef _MARKDOWN_H
#define _MARKDOWN_H

#include <functional>
#include <memory>
#include <sstream>
#include <string>
#include <cctype>
#include <regex>
#include <algorithm>

EOF
cat /tmp/maddy/include/maddy/*.h > /tmp/markdown.2
sed -i -e 's/^#include.*$//' /tmp/markdown.2
sed -i -e 's/^\/\/ .*$//' /tmp/markdown.2
sed -i -e 's/^[[:space:]]\/\/ .*$//' /tmp/markdown.2
sed -i -e 's/^namespace maddy.*$//' /tmp/markdown.2
sed -i -e 's/^} \/\/ namespace maddy.*$//' /tmp/markdown.2
sed -i -e 's/^\/\/ -*$//' /tmp/markdown.2
sed -i -e 's/^#pragma once.*$//' /tmp/markdown.2
sed -i -e 's|/\*|\n&|g;s|*/|&\n|g' /tmp/markdown.2
sed -i -e '/\/\*/,/*\//d' /tmp/markdown.2
sed -i '/^[[:space:]]*$/d' /tmp/markdown.2
sed -i '/maddy::/d' /tmp/markdown.2

cat /tmp/markdown.1 /tmp/markdown.2 > /tmp/markdown.h

cat <<EOF>> /tmp/markdown.h

#endif
EOF

mv /tmp/markdown.h $CDIR/markdown.hpp