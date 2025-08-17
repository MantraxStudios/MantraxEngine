<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
</head>
<body>
<h1>🖥️ Windows Build Instructions</h1>

<h2>✅ Requirements</h2>
<p>Before building the project, ensure you have the following installed:</p>

<ol>
<li><strong>Python 3.6+</strong> 🐍<br>
   - Download from <a href="https://www.python.org/downloads/">python.org</a><br>
   - Make sure <code>python</code> is added to your system PATH.
</li>

<li><strong>Microsoft Visual Studio</strong> 🏗️<br>
   - Install <strong>"Desktop development with C++"</strong> workload.<br>
   - Ensure support for <strong>C++17</strong> is enabled.<br>
   - MSBuild will be used to compile the C++ code.
</li>

<li><strong>CMake</strong> 🔧<br>
   - Required to generate project files for Visual Studio.<br>
   - Download from <a href="https://cmake.org/download/">cmake.org</a><br>
   - Add <code>cmake</code> to your PATH.
</li>
</ol>

<h2>🏗️ Building on Windows</h2>
<ol>
<li>Open a <strong>Developer Command Prompt for Visual Studio</strong> (important so MSBuild is available).</li>
<li>Navigate to the project root where <code>build.py</code> is located:<br>
<pre><code>cd C:\path\to\project</code></pre>
</li>
<li>Run the main build script:<br>
<pre><code>py build.py</code></pre>
<ul>
<li>Automatically detects your platform and executes the appropriate build steps.</li>
<li>Generates Visual Studio project files using <strong>CMake</strong>.</li>
<li>Invokes <strong>MSBuild</strong> to compile all <strong>C++17</strong> targets.</li>
<li>Runs any Python-related setup automatically.</li>
</ul>
</li>
<li>After compilation, binaries will be located in the <code>bin</code> folders created by the build script.</li>
</ol>

<h2>📌 Notes</h2>
<ul>
<li>Ensure your Python version is 3.6 or higher:<br>
<pre><code>py --version</code></pre>
</li>
<li>Make sure you are running the script inside the <strong>Developer Command Prompt</strong>.</li>
<li><strong>C++17</strong> is required; verify your Visual Studio version supports it (Visual Studio 2017 or newer).</li>
<li>You can add additional badges like build status or license at the top of your README for extra visibility.</li>
</ul>

</body>
</html>
