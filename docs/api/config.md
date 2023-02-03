---
layout: default
title: Configuration API
parent: API Documentation
nav_order: 2
---

# Configuration
{:toc}

The `data crunching` library may be configured with the below listed feature-set macros:

| Feature-Set Macro | Description |
|:-----------------:|:------------|
| `DACR_DISABLE_FIELD_LITERAL` | When set, disables the definition of the `_field` literal for `NamedTuple`. This configuration is helpful if the `_field` literal is already defined in your project. |