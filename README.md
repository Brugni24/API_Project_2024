# Pasticceria Manager — Final Project for Algorithms and Data Structures

This project was developed as part of the "Algoritmi e Strutture Dati" (Algorithms and Data Structures) course at Politecnico di Milano during the 2023–2024 academic year. The application is written in C and simulates the internal operations of an industrial pastry shop, handling recipes, ingredient stocks, customer orders, and periodic courier shipments.

## Project Description

The software simulates a pastry shop's logistics in discrete time steps. It manages:

- Recipes, each requiring specific amounts of multiple ingredients
- Ingredient inventory, updated via scheduled supplier shipments with expiration dates
- Customer orders, processed instantly if ingredients are available, or queued otherwise
- Courier pickups, with constraints on weight capacity and order shipment rules

The goal is to accurately simulate the fulfillment pipeline while handling edge cases such as expired inventory, order backlogs, and courier constraints.

## Features

- Add/remove recipes
- Replenish ingredients with expiration dates
- Handle customer orders and queue management
- Smart order fulfillment based on stock availability and FIFO logic
- Courier loading with prioritization by weight and timestamp
- Fully CLI-based simulation using commands provided via input file

## Author

- This project was developed by **Andrea Brugnera** as an individual submission for the final exam.
- **Final grade: 30/30**

## Licence

This repository is intended only as a portfolio showcase.
All rights reserved — copying, redistributing, or using the code is not allowed without explicit permission.
