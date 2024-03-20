Code Styles
-----------

To apply style guidelines, `astyle <https://astyle.sourceforge.net/>`_ is used.

For the source code in the repository, the following configuration should be used
through astyle_config file:

.. code-block:: bash
    
    --style=linux
    --indent=force-tab=8
    --max-code-length=80
    --suffix=none

**Run manually:**

To use it, copy the script above to a file named e.g. 'astyle_config'. Then, to 
apply the style to the source file example.cpp, call 'astyle' with the following
command: 

.. code-block:: bash

    astyle --options=astyle_config example.cpp

The file example.cpp will be rewritten to match the style rules.

**Run from script (Linux):**

A shell script that automates the code style check is available at:
:git-no-os:`ci/astyle.sh`.

Run the script from the root of the repository, for example:

.. code-block:: bash
    
    ./ci/astyle.sh

All the changed files in the last commit from the current branch will be checked
and rewritten. The differences will be displayed after code style is applied.

**Run script with parameter:**

Optionally, the commit range can be passed as parameter to the script, applying 
code style on all changed files within the commits.

.. code-block:: bash
    
    ./ci/astyle.sh <COMMIT_RANGE>
