## W1m mirror covers daemon

`coversd` interfaces with and wraps a microcontroller relay board (see controller directory) that drive the linear actuators.

`covers` is a commandline utility for controlling the covers.

### Configuration

Configuration is read from json files that are installed by default to `/etc/coversd`.
A configuration file is specified when launching the server, and the `covers` frontend will search this location when launched.

The configuration options are:
```python
{
  "daemon": "onemetre_covers", # Run the server as this daemon. Daemon types are registered in `rockit.common.daemons`.
  "log_name": "coversd@onemetre", # The name to use when writing messages to the observatory log.
  "control_machines": ["OneMetreDome", "OneMetreTCS"], # Machine names that are allowed to control (rather than just query) state. Machine names are registered in `rockit.common.IP`.
  "serial_port": "/dev/covers",
  "serial_baud": 9600,
  "serial_timeout": 5,
  "move_timeout": 30
}

```
## Initial Installation

The automated packaging scripts will push 4 RPM packages to the observatory package repository:

| Package                     | Description                                                                  |
|-----------------------------|------------------------------------------------------------------------------|
| rockit-covers-server        | Contains the `coversd` server and systemd service file.                      |
| rockit-covers-client        | Contains the `covers` commandline utility for controlling the covers server. |
| python3-rockit-covers       | Contains the python module with shared code.                                 |
| rockit-covers-data-onemetre | Contains the json configuration for the W1m.                                 |

After installing packages, the systemd service should be enabled:

```
sudo systemctl enable --now coversd@<config>
```

where `config` is the name of the json file for the appropriate telescope.

Now open a port in the firewall:
```
sudo firewall-cmd --zone=public --add-port=<port>/tcp --permanent
sudo firewall-cmd --reload
```
where `port` is the port defined in `rockit.common.daemons` for the daemon specified in the config.

### Upgrading Installation

New RPM packages are automatically created and pushed to the package repository for each push to the `master` branch.
These can be upgraded locally using the standard system update procedure:
```
sudo yum clean expire-cache
sudo yum update
```

The daemon should then be restarted to use the newly installed code:
```
sudo systemctl restart coversd@<config>
```

### Testing Locally

The camera server and client can be run directly from a git clone:
```
./coversd onemetre.json
COVERSD_CONFIG_PATH=./onemetre.json ./covers status
```
